// The web build's page logic. The page is only the game: a build made beside
// a local game copy packages it (game.data, cmake/web.cmake) and the game
// starts at once. Otherwise one button asks for the player's game folder,
// which is kept in IndexedDB so it is chosen only once. Saves (%APPDATA%)
// persist in IndexedDB as well. Nothing is uploaded.
'use strict';

var GAME = '/game';          // the game folder: the working directory
var SAVE = '/save';          // %APPDATA%: profiles and settings
var RESOURCES = GAME + '/.vc1/platform-resources';
var READY = GAME + '/.vc1/ready';
// Whether the build packaged a local game copy (game.data.js beside the page).
var packaged = false;

var pick = document.getElementById('pick');

// F12 and the devtools shortcuts belong to the browser: SDL would take them
// as game keys and cancel them. This capture listener runs before SDL's.
function passBrowserKeys(event) {
  if (event.key === 'F12' || (event.ctrlKey && event.shiftKey && /^[ijc]$/i.test(event.key))) {
    event.stopImmediatePropagation();
  }
}
window.addEventListener('keydown', passBrowserKeys, true);
window.addEventListener('keyup', passBrowserKeys, true);

function sync(populate) {
  return new Promise(function (resolve, reject) {
    FS.syncfs(populate, function (error) {
      if (error) {
        reject(error);
      } else {
        resolve();
      }
    });
  });
}

function mkdirs(path) {
  var parts = path.split('/');
  var current = '';
  for (var i = 1; i < parts.length; ++i) {
    current += '/' + parts[i];
    if (!FS.analyzePath(current).exists) {
      FS.mkdir(current);
    }
  }
}

function removeTree(path) {
  FS.readdir(path).forEach(function (name) {
    if (name === '.' || name === '..') {
      return;
    }
    var child = path + '/' + name;
    if (FS.isDir(FS.stat(child).mode)) {
      removeTree(child);
      FS.rmdir(child);
    } else {
      FS.unlink(child);
    }
  });
}

// The original's cursors and icon (resource types 12/1 and 14/3), converted
// to the VCR1 files user32.cpp loads, as tools/extract-sdl-resources.py does.
function extractResources(bytes) {
  var view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  var pe = view.getUint32(0x3c, true);
  if (view.getUint32(pe, true) !== 0x4550) {
    return 0;
  }
  var sectionCount = view.getUint16(pe + 6, true);
  var optional = pe + 24;
  var optionalSize = view.getUint16(pe + 20, true);
  var resourceRva = view.getUint32(optional + 96 + 2 * 8, true);
  var sections = optional + optionalSize;
  function offsetOf(rva) {
    for (var i = 0; i < sectionCount; ++i) {
      var s = sections + i * 40;
      var va = view.getUint32(s + 12, true);
      var size = Math.max(view.getUint32(s + 8, true), view.getUint32(s + 16, true));
      if (rva >= va && rva < va + size) {
        return rva - va + view.getUint32(s + 20, true);
      }
    }
    return -1;
  }
  var root = offsetOf(resourceRva);
  if (!resourceRva || root < 0) {
    return 0;
  }
  function entries(directory) {
    var out = [];
    var count = view.getUint16(directory + 12, true) + view.getUint16(directory + 14, true);
    for (var i = 0; i < count; ++i) {
      var e = directory + 16 + i * 8;
      out.push({ name: view.getUint32(e, true), offset: view.getUint32(e + 4, true) });
    }
    return out;
  }
  var resources = {};
  entries(root).forEach(function (type) {
    if ([1, 3, 12, 14].indexOf(type.name) < 0 || !(type.offset & 0x80000000)) {
      return;
    }
    entries(root + (type.offset & 0x7fffffff)).forEach(function (item) {
      if ((item.name & 0x80000000) || !(item.offset & 0x80000000)) {
        return;
      }
      var language = entries(root + (item.offset & 0x7fffffff))[0];
      if (!language || (language.offset & 0x80000000)) {
        return;
      }
      var data = root + language.offset;
      var at = offsetOf(view.getUint32(data, true));
      resources[type.name + ':' + item.name] = bytes.subarray(at, at + view.getUint32(data + 4, true));
    });
  });
  var written = 0;
  [[12, 1, 'cursor'], [14, 3, 'icon']].forEach(function (kind) {
    Object.keys(resources).forEach(function (key) {
      var parts = key.split(':');
      if (+parts[0] !== kind[0]) {
        return;
      }
      var group = resources[key];
      var g = new DataView(group.buffer, group.byteOffset, group.byteLength);
      var best = null;
      for (var i = 0, n = g.getUint16(4, true); i < n; ++i) {
        var data = resources[kind[1] + ':' + g.getUint16(6 + i * 14 + 12, true)];
        if (!data) {
          continue;
        }
        var d = new DataView(data.buffer, data.byteOffset, data.byteLength);
        var hotX = kind[1] === 1 ? d.getUint16(0, true) : 0;
        var hotY = kind[1] === 1 ? d.getUint16(2, true) : 0;
        var dib = kind[1] === 1 ? 4 : 0;
        if (data.length - dib < 40 || d.getUint32(dib, true) !== 40 || d.getUint16(dib + 14, true) !== 32) {
          continue;
        }
        var width = d.getInt32(dib + 4, true);
        var doubleHeight = d.getInt32(dib + 8, true);
        var height = Math.abs(doubleHeight) >> 1;
        var stride = width * 4;
        var maskStride = ((width + 31) >> 5) * 4;
        var pixelsAt = dib + 40;
        var alpha = false;
        for (var a = pixelsAt + 3; a < pixelsAt + stride * height; a += 4) {
          if (data[a]) {
            alpha = true;
            break;
          }
        }
        var pixels = new Uint8Array(width * height * 4);
        for (var y = 0; y < height; ++y) {
          var sy = doubleHeight > 0 ? height - 1 - y : y;
          for (var x = 0; x < width; ++x) {
            var from = pixelsAt + sy * stride + x * 4;
            var to = (y * width + x) * 4;
            pixels.set(data.subarray(from, from + 4), to);
            if (!alpha) {
              var mask = pixelsAt + stride * height + sy * maskStride + (x >> 3);
              pixels[to + 3] = mask < data.length && (data[mask] & (0x80 >> (x & 7))) ? 0 : 255;
            }
          }
        }
        var score = Math.abs(width - 32);
        if (!best || score < best.score) {
          best = { score: score, width: width, height: height, hotX: hotX, hotY: hotY, pixels: pixels };
        }
      }
      if (best) {
        var file = new Uint8Array(20 + best.pixels.length);
        var h = new DataView(file.buffer);
        file.set([0x56, 0x43, 0x52, 0x31]);
        h.setUint32(4, best.width, true);
        h.setUint32(8, best.height, true);
        h.setUint32(12, best.hotX, true);
        h.setUint32(16, best.hotY, true);
        file.set(best.pixels, 20);
        FS.writeFile(RESOURCES + '/' + kind[2] + parts[1] + '.bin', file);
        ++written;
      }
    });
  });
  return written;
}

// The game folder holds VirtualCity.bar, the 7z archive the game mounts from
// its working directory (the shipped layout); only the files beside it are
// copied. A folder of extracted files (scripts/ and res/) works as well. The
// selection may be either folder or a parent; any .exe in it gives the
// cursors. Each entry is [relative path, Blob].
async function importFolder(files) {
  var root = null;
  for (var b = 0; b < files.length && root === null; ++b) {
    if (/\.bar$/i.test(files[b][0])) {
      root = files[b][0].substring(0, files[b][0].lastIndexOf('/'));
    }
  }
  var archive = root !== null;
  for (var i = 0; i < files.length && root === null; ++i) {
    var parts = files[i][0].split('/');
    for (var j = 0; j < parts.length - 1; ++j) {
      if (parts[j].toLowerCase() === 'scripts') {
        root = parts.slice(0, j).join('/');
        break;
      }
    }
  }
  if (root === null) {
    throw new Error('No VirtualCity.bar in that selection. Choose the Virtual City game folder.');
  }
  var prefix = root + '/';
  var chosen = [];
  var executables = [];
  for (var k = 0; k < files.length; ++k) {
    var path = files[k][0];
    var inside = path.indexOf(prefix) === 0 &&
      (!archive || path.indexOf('/', prefix.length) < 0);
    if (inside) {
      chosen.push([path.substring(prefix.length), files[k][1]]);
    }
    if (/\.exe$/i.test(path)) {
      executables.push(files[k][1]);
    }
  }
  if (FS.analyzePath(GAME + '/.vc1').exists || FS.readdir(GAME).length > 2) {
    removeTree(GAME);
  }
  for (var n = 0; n < chosen.length; ++n) {
    var target = GAME + '/' + chosen[n][0];
    mkdirs(target.substring(0, target.lastIndexOf('/')));
    FS.writeFile(target, new Uint8Array(await chosen[n][1].arrayBuffer()));
    if (n % 25 === 0) {
      pick.textContent = Math.floor(100 * n / chosen.length) + '%';
    }
  }
  mkdirs(RESOURCES);
  for (var e = 0; e < executables.length; ++e) {
    try {
      if (extractResources(new Uint8Array(await executables[e].arrayBuffer()))) {
        break;
      }
    } catch (error) {
      console.warn('No resources in an .exe of the selection', error);
    }
  }
  FS.writeFile(READY, root || '.');
  await sync(false);
}

function start() {
  pick.hidden = true;
  if (FS.analyzePath('/platform-resources').exists) {
    FS.unlink('/platform-resources');
  }
  FS.symlink(RESOURCES, '/platform-resources');
  FS.chdir(GAME);
  // Saves are written as the game goes; keep IndexedDB current.
  setInterval(function () {
    FS.syncfs(false, function () {});
  }, 5000);
  document.getElementById('canvas').focus();
  Module.callMain([]);
}

pick.onclick = function () {
  document.getElementById('folder').click();
};

document.getElementById('folder').onchange = async function (event) {
  var files = Array.prototype.map.call(event.target.files, function (file) {
    return [file.webkitRelativePath, file];
  });
  var label = pick.textContent;
  pick.disabled = true;
  try {
    if (navigator.storage && navigator.storage.persist) {
      navigator.storage.persist();
    }
    await importFolder(files);
    start();
  } catch (error) {
    console.error(error);
    alert(String(error.message || error));
    pick.textContent = label;
    pick.disabled = false;
  }
  event.target.value = '';
};

var Module = {
  canvas: document.getElementById('canvas'),
  preRun: [function () {
    // A packaged build fills /game from game.data (game.data.js).
    if (!packaged) {
      FS.mkdir(GAME);
      FS.mount(IDBFS, {}, GAME);
    }
    FS.mkdir(SAVE);
    FS.mount(IDBFS, {}, SAVE);
    ENV.APPDATA = SAVE;
    Module.addRunDependency('vc1-storage');
    FS.syncfs(true, function (error) {
      if (error) {
        console.error('IndexedDB', error);
      }
      Module.removeRunDependency('vc1-storage');
    });
  }],
  onRuntimeInitialized: function () {
    if (packaged || FS.analyzePath(READY).exists) {
      start();
    } else {
      pick.hidden = false;
    }
  },
  onExit: function () {
    FS.syncfs(false, function () {});
  },
  print: function (text) {
    console.log(text);
  },
  printErr: function (text) {
    console.warn(text);
  }
};

function loadScript(src) {
  return new Promise(function (resolve, reject) {
    var script = document.createElement('script');
    script.src = src;
    script.async = false;
    script.onload = resolve;
    script.onerror = reject;
    document.body.appendChild(script);
  });
}

// game.data.js must register its preload before the module starts.
(async function () {
  try {
    packaged = (await fetch('game.data.js', { method: 'HEAD' })).ok;
  } catch (error) {
    packaged = false;
  }
  if (packaged) {
    await loadScript('game.data.js');
  }
  await loadScript('VirtualCity.js');
})();
