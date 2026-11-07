// The control panel: F10 or the corner button. Game (reload, pause, speed,
// sound), Saves (back up, import, edit), Cheats (the game's own G5CHTR keys),
// Console (the log and Squirrel lines, as the engine's console runs them:
// the game itself never creates that console) and Mods
// (texture mods in IndexedDB). It talks to the game through bridge.cpp:
// run(code, argument) queues a Squirrel script for the next frame and
// resolves with its result.
'use strict';

(function () {
  var MODS = '/save/mods';
  var LOG_LINES = 5000;

  // ---- The game ------------------------------------------------------------

  var started = false;
  var initialized = Module.onRuntimeInitialized;
  Module.onRuntimeInitialized = function () {
    started = true;
    initialized();
  };

  var requests = {};
  var nextRequest = 1;

  Module.vc1Result = function (id, ok, text) {
    var request = requests[id];
    delete requests[id];
    if (request) {
      if (ok) {
        request.resolve(text);
      } else {
        request.reject(new Error(text));
      }
    }
  };

  // Runs a Squirrel script in the game, root table as "this", ::vc1_arg set
  // to argument. Resolves with the returned value as text.
  function run(code, argument) {
    if (!started) {
      return Promise.reject(new Error('the game is still loading'));
    }
    return helpers().then(function () {
      return rawRun(code, argument);
    });
  }

  function rawRun(code, argument) {
    return new Promise(function (resolve, reject) {
      var id = nextRequest++;
      requests[id] = { resolve: resolve, reject: reject };
      Module.ccall('vc1_web_run', null, ['number', 'string', 'string'], [id, code, argument || '']);
    });
  }

  // Script helpers, installed once: vc1_json writes a value as JSON with its
  // Squirrel type ({"t": type, "v": value}; a table's v lists [key, value]),
  // vc1_write saves a table as ProfileInfo.SaveTable does.
  var HELPERS = [
    '::vc1_escape <- function (s) {',
    '  local r = "", start = 0, hex = "0123456789abcdef";',
    '  for (local i = 0; i < s.len(); i++) {',
    '    local c = s[i];',
    '    if (c == 34 || c == 92 || c < 32) {',
    '      r += s.slice(start, i) + "\\\\u00" + hex.slice(c / 16, c / 16 + 1) + hex.slice(c % 16, c % 16 + 1);',
    '      start = i + 1;',
    '    }',
    '  }',
    '  return r + s.slice(start);',
    '}',
    '::vc1_json <- function (v) {',
    '  local t = typeof v;',
    '  if (t == "table") {',
    '    local s = "", first = true;',
    '    foreach (k, x in v) { s += (first ? "" : ",") + "[" + ::vc1_json(k) + "," + ::vc1_json(x) + "]"; first = false; }',
    '    return "{\\"t\\":\\"table\\",\\"v\\":[" + s + "]}";',
    '  }',
    '  if (t == "array") {',
    '    local s = "", first = true;',
    '    foreach (x in v) { s += (first ? "" : ",") + ::vc1_json(x); first = false; }',
    '    return "{\\"t\\":\\"array\\",\\"v\\":[" + s + "]}";',
    '  }',
    '  if (t == "string") return "{\\"t\\":\\"string\\",\\"v\\":\\"" + ::vc1_escape(v) + "\\"}";',
    '  if (t == "integer" || t == "float" || t == "bool") return "{\\"t\\":\\"" + t + "\\",\\"v\\":" + v + "}";',
    '  return "{\\"t\\":\\"" + t + "\\"}";',
    '}',
    '::vc1_write <- function (path, table) {',
    '  local text = ::createScriptString(table);',
    '  if (::RETAILER_CODE == "none") ::saveStringToFile(path, text);',
    '  else ::writeclosuretofile(path, ::compilestring(text));',
    '}',
    'return "";'
  ].join('\n');
  var helpersReady = null;

  function helpers() {
    if (!helpersReady) {
      helpersReady = rawRun(HELPERS).catch(function (error) {
        helpersReady = null;
        throw error;
      });
    }
    return helpersReady;
  }

  // A Squirrel string literal.
  function literal(text) {
    return '"' + String(text).replace(/[\\"\x00-\x1f]/g, function (c) {
      var code = c.charCodeAt(0);
      if (c === '\\' || c === '"') {
        return '\\' + c;
      }
      return '\\x' + (code < 16 ? '0' : '') + code.toString(16);
    }) + '"';
  }

  function syncSaves() {
    return new Promise(function (resolve) {
      FS.syncfs(false, function (error) {
        if (error) {
          console.error('IndexedDB', error);
        }
        resolve();
      });
    });
  }

  function reload() {
    syncSaves().then(function () {
      location.reload();
    });
  }

  // The game writes its profiles only when a level is completed and when it
  // quits, which a page never does: save them first.
  function saveGame() {
    return run('::ProfileInfo.Save(); return "";').then(syncSaves);
  }

  function saveAndReload() {
    saveGame().then(function () {
      location.reload();
    }, reload);
  }

  // The page's audio contexts (SDL's), so pause and mute can suspend them.
  var audioContexts = [];
  ['AudioContext', 'webkitAudioContext'].forEach(function (name) {
    var Base = window[name];
    if (!Base) {
      return;
    }
    var Tracked = function (options) {
      var context = new Base(options);
      audioContexts.push(context);
      if (paused || muted) {
        context.suspend();
      }
      return context;
    };
    Tracked.prototype = Base.prototype;
    window[name] = Tracked;
  });

  var paused = false;
  var muted = false;

  function updateAudio() {
    // audio.cpp resumes the output on the first click or key unless held.
    Module.vc1AudioHeld = paused || muted;
    audioContexts.forEach(function (context) {
      if (paused || muted) {
        context.suspend();
      } else {
        context.resume();
      }
    });
  }

  // ---- Page elements -------------------------------------------------------

  function element(tag, attributes, children) {
    var node = document.createElement(tag);
    Object.keys(attributes || {}).forEach(function (key) {
      if (key === 'text') {
        node.textContent = attributes[key];
      } else if (key.slice(0, 2) === 'on') {
        node.addEventListener(key.slice(2), attributes[key]);
      } else {
        node.setAttribute(key, attributes[key]);
      }
    });
    (children || []).forEach(function (child) {
      if (child) {
        node.appendChild(typeof child === 'string' ? document.createTextNode(child) : child);
      }
    });
    return node;
  }

  function button(text, action, title) {
    return element('button', { type: 'button', text: text, title: title || '', onclick: action });
  }

  var status = element('div', { 'class': 'vc1-status' });
  var statusTimer = 0;

  function say(text, error) {
    status.textContent = text;
    status.className = 'vc1-status' + (error ? ' vc1-error' : '');
    clearTimeout(statusTimer);
    statusTimer = setTimeout(function () {
      status.textContent = '';
    }, error ? 8000 : 3000);
  }

  function failed(error) {
    say(error.message || String(error), true);
  }

  function download(name, bytes) {
    var link = element('a', { href: URL.createObjectURL(new Blob([bytes])), download: name });
    document.body.appendChild(link);
    link.click();
    link.remove();
    setTimeout(function () {
      URL.revokeObjectURL(link.href);
    }, 1000);
  }

  function pickFiles(options, handle) {
    var input = element('input', { type: 'file' });
    if (options.accept) {
      input.accept = options.accept;
    }
    if (options.folder) {
      input.webkitdirectory = true;
    }
    input.addEventListener('change', function () {
      if (input.files.length) {
        handle(Array.prototype.slice.call(input.files));
      }
    });
    input.click();
  }

  function exists(path) {
    try {
      FS.stat(path);
      return true;
    } catch (error) {
      return false;
    }
  }

  function makeTree(path) {
    var at = '';
    path.split('/').filter(Boolean).forEach(function (part) {
      at += '/' + part;
      if (!exists(at)) {
        FS.mkdir(at);
      }
    });
  }

  function removeTree(path) {
    if (FS.isDir(FS.stat(path).mode)) {
      FS.readdir(path).forEach(function (name) {
        if (name !== '.' && name !== '..') {
          removeTree(path + '/' + name);
        }
      });
      FS.rmdir(path);
    } else {
      FS.unlink(path);
    }
  }

  // ---- Game tab ------------------------------------------------------------

  function gameTab() {
    var pause = button('Pause', function () {
      paused = !paused;
      Module.ccall('vc1_web_pause', null, ['number'], [paused ? 1 : 0]);
      pause.textContent = paused ? 'Resume' : 'Pause';
      updateAudio();
    });
    var mute = button('Mute', function () {
      muted = !muted;
      mute.textContent = muted ? 'Unmute' : 'Mute';
      updateAudio();
    });
    var speed = element('span', { 'class': 'vc1-value', text: '' });
    function showSpeed() {
      run('return ::System.GetUpdateSpeed();').then(function (value) {
        speed.textContent = Number(value) + '×';
      }, failed);
    }
    function setSpeed(value) {
      run('::System.SetUpdateSpeed(' + value.toFixed(4) + '); return "";').then(showSpeed, failed);
    }
    var speeds = [0.25, 0.5, 1, 2, 4, 8].map(function (value) {
      return button(value + '×', function () {
        setSpeed(value);
      });
    });
    return {
      name: 'Game',
      shown: showSpeed,
      body: [
        element('div', { 'class': 'vc1-row' }, [
          button('Reload', saveAndReload, 'Save the profiles and restart the game'),
          pause,
          mute,
          button('Fullscreen', function () {
            document.documentElement.requestFullscreen().catch(failed);
          })
        ]),
        element('h3', { text: 'Game speed' }),
        element('div', { 'class': 'vc1-row' }, speeds.concat([speed])),
        element('p', { 'class': 'vc1-note', text: 'Levels reset the speed to 1× when they start.' })
      ]
    };
  }

  // ---- Saves tab -----------------------------------------------------------

  // The game keeps its profiles in data/ProfileInfo_<n>.nut, which the KD
  // file layer puts in %APPDATA%/Virtual City.
  var SAVE_DIR = '/save/Virtual City';

  function saveFiles() {
    if (!exists(SAVE_DIR)) {
      return [];
    }
    return FS.readdir(SAVE_DIR).filter(function (name) {
      return /\.nut$/i.test(name);
    });
  }

  function savesTab() {
    var list = element('div');
    var editor = element('div', { 'class': 'vc1-editor' });

    function refresh() {
      list.textContent = '';
      var files = saveFiles();
      if (!files.length) {
        list.appendChild(element('p', { 'class': 'vc1-note', text: 'No saves yet. The game saves after each level; Save now writes the current profiles.' }));
      }
      files.forEach(function (name) {
        var path = SAVE_DIR + '/' + name;
        list.appendChild(element('div', { 'class': 'vc1-row vc1-file' }, [
          element('span', { 'class': 'vc1-grow', text: name + ' (' + FS.stat(path).size + ' bytes)' }),
          button('Edit', function () {
            edit(name);
          }),
          button('Download', function () {
            download(name, FS.readFile(path));
          }),
          button('Delete', function () {
            if (confirm('Delete ' + name + '? The game starts with new profiles after the reload.')) {
              FS.unlink(path);
              reload();
            }
          })
        ]));
      });
    }

    function importSave() {
      pickFiles({ accept: '.nut' }, function (files) {
        var file = files[0];
        file.arrayBuffer().then(function (buffer) {
          var name = /^ProfileInfo_\d+\.nut$/i.test(file.name) ? file.name : 'ProfileInfo_0.nut';
          var path = SAVE_DIR + '/' + name;
          if (exists(path) && !confirm('Replace ' + name + ' with ' + file.name + '?')) {
            return;
          }
          makeTree(SAVE_DIR);
          FS.writeFile(path, new Uint8Array(buffer));
          reload();
        }).catch(failed);
      });
    }

    function edit(name) {
      var gamePath = literal('data/' + name);
      editor.textContent = '';
      editor.appendChild(element('p', { 'class': 'vc1-note', text: 'Reading ' + name + '…' }));
      run('return ::vc1_json(::dofile(' + gamePath + '));').then(function (json) {
        showEditor(name, gamePath, JSON.parse(json));
      }).catch(failed);
    }

    // The save as a tree of editable values, and as script text.
    function showEditor(name, gamePath, tree) {
      var edits = [];
      editor.textContent = '';

      function leaf(node, path) {
        var input;
        if (node.t === 'bool') {
          input = element('input', { type: 'checkbox' });
          input.checked = node.v;
        } else {
          input = element('input', { type: node.t === 'string' ? 'text' : 'number', step: 'any' });
          input.value = node.v;
        }
        input.addEventListener('change', function () {
          var value;
          if (node.t === 'bool') {
            value = input.checked ? 'true' : 'false';
          } else if (node.t === 'string') {
            value = literal(input.value);
          } else if (node.t === 'integer') {
            value = String(Math.trunc(Number(input.value) || 0));
          } else {
            value = String(Number(input.value) || 0);
            if (!/[.e]/.test(value)) {
              value += '.0';
            }
          }
          edits = edits.filter(function (edit) {
            return edit.path !== path;
          });
          edits.push({ path: path, value: value });
          apply.disabled = false;
        });
        return input;
      }

      function keyText(node) {
        return node.t === 'string' ? node.v : String(node.v);
      }

      function keyCode(node) {
        return '[' + (node.t === 'string' ? literal(node.v) : String(node.v)) + ']';
      }

      function branch(label, node, path) {
        var entries = node.t === 'table' ? node.v.map(function (pair) {
          return { label: keyText(pair[0]), path: path + keyCode(pair[0]), node: pair[1] };
        }) : node.v.map(function (child, index) {
          return { label: String(index), path: path + '[' + index + ']', node: child };
        });
        entries.sort(function (a, b) {
          return a.label.localeCompare(b.label, undefined, { numeric: true });
        });
        var details = element('details', {}, [
          element('summary', { text: label + ' ' + (node.t === 'table' ? '{' + entries.length + '}' : '[' + entries.length + ']') })
        ]);
        var filled = false;
        details.addEventListener('toggle', function () {
          if (!filled && details.open) {
            filled = true;
            entries.forEach(function (entry) {
              details.appendChild(item(entry.label, entry.node, entry.path));
            });
          }
        });
        return details;
      }

      function item(label, node, path) {
        if (node.t === 'table' || node.t === 'array') {
          return branch(label, node, path);
        }
        if (node.t === 'string' || node.t === 'integer' || node.t === 'float' || node.t === 'bool') {
          return element('label', { 'class': 'vc1-leaf' }, [element('span', { text: label }), leaf(node, path)]);
        }
        return element('div', { 'class': 'vc1-leaf' }, [element('span', { text: label }), element('i', { text: node.t })]);
      }

      var apply = button('Save changes', function () {
        var code = ['local t = ::dofile(' + gamePath + ');'];
        edits.forEach(function (edit) {
          code.push('t' + edit.path + ' = ' + edit.value + ';');
        });
        code.push('::vc1_write(' + gamePath + ', t);', 'return "";');
        run(code.join('\n')).then(function () {
          say('Saved; reloading the game');
          reload();
        }, failed);
      });
      apply.disabled = true;

      var text = element('textarea', { 'class': 'vc1-text', spellcheck: 'false' });
      var saveText = button('Save text', function () {
        run('::vc1_write(' + gamePath + ', ::compilestring(::vc1_arg)()); return "";', text.value).then(function () {
          say('Saved; reloading the game');
          reload();
        }, failed);
      });
      var textView = element('details', {}, [element('summary', { text: 'As script text' }), text, saveText]);
      textView.addEventListener('toggle', function () {
        if (textView.open && !text.value) {
          run('return ::createScriptString(::dofile(' + gamePath + '));').then(function (script) {
            text.value = script;
          }, failed);
        }
      });

      var root = branch(name, tree, '');
      root.open = true;
      root.dispatchEvent(new Event('toggle'));
      editor.appendChild(element('div', { 'class': 'vc1-row' }, [
        apply,
        button('Close', function () {
          editor.textContent = '';
        })
      ]));
      editor.appendChild(element('p', { 'class': 'vc1-note', text: 'Saving writes the file and reloads the game, which reads it at start.' }));
      editor.appendChild(root);
      editor.appendChild(textView);
    }

    return {
      name: 'Saves',
      shown: refresh,
      body: [
        list,
        element('div', { 'class': 'vc1-row' }, [
          button('Save now', function () {
            saveGame().then(function () {
              say('Saved');
              refresh();
            }, failed);
          }, 'The game itself saves only after a level'),
          button('Import a save', importSave),
          button('Back up all', function () {
            saveFiles().forEach(function (name) {
              download(name, FS.readFile(SAVE_DIR + '/' + name));
            });
          })
        ]),
        editor
      ]
    };
  }

  // ---- Cheats tab ----------------------------------------------------------

  // The game's own cheats (Game.nut, Level.nut, menus/Map.nut): typing G5CHTR
  // sets ::IsCheatsEnabled and the keys below act. They are sent through
  // ::Game.OnKeyPressed as if typed.
  function pressKey(key, release) {
    var code = '::IsCheatsEnabled = true;\n::Game.OnKeyPressed(::KeyCodes.' + key + ', 0);\n';
    if (release !== false) {
      code += '::Game.OnKeyReleased(::KeyCodes.' + key + ', 0);\n';
    }
    return run(code + 'return "";');
  }

  // The map's cheats; in a level the same keys complete it.
  function onMap(key) {
    return run('if (::Level != null) throw "only on the map screen"; return "";').then(function () {
      return pressKey(key);
    });
  }

  function inLevel(code) {
    return run('if (::Level == null) throw "only in a level";\n' + code);
  }

  function cheatsTab() {
    var amount = element('input', { type: 'number', value: '10000', step: '1000' });
    var debug = false;
    var debugButton = button('Debug overlay', function () {
      var code = debug ? '::Game.OnKeyReleased(::KeyCodes.SHIFT, 0);' : '::IsCheatsEnabled = true; ::Game.OnKeyPressed(::KeyCodes.SHIFT, 0);';
      inLevel(code + ' return "";').then(function () {
        debug = !debug;
        debugButton.textContent = debug ? 'Hide debug overlay' : 'Debug overlay';
      }, failed);
    });
    function done(text) {
      return function () {
        say(text);
      };
    }
    return {
      name: 'Cheats',
      body: [
        element('h3', { text: 'In a level' }),
        element('div', { 'class': 'vc1-row' }, [
          amount,
          button('Add money', function () {
            inLevel('::Level.ChangeMoney(' + Math.trunc(Number(amount.value) || 0) + '); return "";').then(done('Money changed'), failed);
          })
        ]),
        element('div', { 'class': 'vc1-row' }, [
          button('Complete level', function () {
            inLevel('::IsCheatsEnabled = true; ::Game.OnKeyPressed(::KeyCodes.STAR, 0); return "";').then(done('Completed (or skipped a tutorial step)'), failed);
          }, 'Skips the current tutorial step first, as the * cheat does'),
          button('Rain', function () {
            inLevel('return "";').then(function () {
              return pressKey('First + 82');
            }).then(done('Rain'), failed);
          }),
          button('Clouds', function () {
            inLevel('return "";').then(function () {
              return pressKey('First + 67');
            }).then(done('Clouds'), failed);
          }),
          debugButton
        ]),
        element('h3', { text: 'On the map' }),
        element('div', { 'class': 'vc1-row' }, [
          button('Unlock all levels', function () {
            onMap('STAR').then(done('Unlocked'), failed);
          }, 'The map’s * cheat: opens every campaign and level'),
          button('Test level', function () {
            onMap('First + 84').then(done('Test level'), failed);
          }, 'The map’s T cheat; elsewhere T resets the tile texture usage')
        ]),
        element('p', { 'class': 'vc1-note', text: 'These are the game’s G5CHTR cheats. Once one is used, the cheat keys also work in game: Space +1000, * complete, +/- speed, R rain, C clouds, Shift debug.' })
      ]
    };
  }

  // ---- Console tab ---------------------------------------------------------

  var logLines = [];
  var logView = null;
  var logFilter = 'all';

  function logClass(text) {
    if (/error/i.test(text)) {
      return 'error';
    }
    if (/warning/i.test(text)) {
      return 'warning';
    }
    return 'info';
  }

  // Lines reach the view in batches: the game logs thousands while loading.
  var unshown = 0;
  var flushTimer = 0;

  function addLog(text, kind) {
    logLines.push({ text: text, kind: kind || logClass(text) });
    unshown++;
    if (logLines.length > LOG_LINES) {
      logLines.shift();
      if (unshown > logLines.length) {
        unshown = logLines.length;
      } else if (logView && logView.firstChild) {
        logView.removeChild(logView.firstChild);
      }
    }
    if (logView && !flushTimer) {
      flushTimer = setTimeout(flushLog, 100);
    }
  }

  function lineNode(line) {
    var node = element('div', { 'class': 'vc1-log-' + line.kind, text: line.text });
    node.hidden = logFilter !== 'all' && line.kind !== logFilter && line.kind !== 'input';
    return node;
  }

  // Shows the lines logged since the last flush, while the panel is open.
  function flushLog() {
    flushTimer = 0;
    if (!logView || !unshown || panel.hidden) {
      return;
    }
    var stick = logView.scrollTop + logView.clientHeight >= logView.scrollHeight - 4;
    var batch = document.createDocumentFragment();
    logLines.slice(logLines.length - unshown).forEach(function (line) {
      batch.appendChild(lineNode(line));
    });
    unshown = 0;
    logView.appendChild(batch);
    while (logView.childNodes.length > logLines.length) {
      logView.removeChild(logView.firstChild);
    }
    if (stick) {
      logView.scrollTop = logView.scrollHeight;
    }
  }

  ['print', 'printErr'].forEach(function (name) {
    var original = Module[name];
    Module[name] = function (text) {
      addLog(text);
      if (original) {
        original(text);
      }
    };
  });

  function consoleTab() {
    var history = [];
    var historyIndex = 0;
    logView = element('div', { 'class': 'vc1-log' });
    var input = element('input', { type: 'text', 'class': 'vc1-grow', placeholder: 'Squirrel, e.g. ::Level.m_Money', spellcheck: 'false' });

    // An expression shows its value (as the game's emitScript writes it);
    // anything else runs as statements.
    function execute() {
      var line = input.value.trim();
      if (!line) {
        return;
      }
      history.push(line);
      historyIndex = history.length;
      input.value = '';
      addLog('> ' + line, 'input');
      run('return ::emitScript("", (' + line + '));').catch(function (error) {
        if (/^compile: /.test(error.message)) {
          return run(line + '\nreturn "";');
        }
        throw error;
      }).then(function (result) {
        if (result) {
          addLog(result, 'result');
        }
      }, function (error) {
        addLog(error.message, 'error');
      });
    }

    input.addEventListener('keydown', function (event) {
      if (event.key === 'Enter') {
        execute();
      } else if (event.key === 'ArrowUp' && historyIndex > 0) {
        input.value = history[--historyIndex];
        event.preventDefault();
      } else if (event.key === 'ArrowDown' && historyIndex < history.length) {
        historyIndex++;
        input.value = history[historyIndex] || '';
        event.preventDefault();
      }
    });

    var filters = ['all', 'warning', 'error'].map(function (kind) {
      var node = button(kind === 'all' ? 'All' : kind === 'warning' ? 'Warnings' : 'Errors', function () {
        logFilter = kind;
        flushLog();
        Array.prototype.forEach.call(logView.children, function (child, index) {
          var line = logLines[index];
          child.hidden = !!line && kind !== 'all' && line.kind !== kind && line.kind !== 'input';
        });
        filters.forEach(function (other) {
          other.classList.toggle('vc1-on', other === node);
        });
      });
      return node;
    });
    filters[0].classList.add('vc1-on');

    return {
      name: 'Console',
      shown: function () {
        flushLog();
        logView.scrollTop = logView.scrollHeight;
        input.focus();
      },
      body: [
        element('div', { 'class': 'vc1-row' }, filters.concat([
          button('Clear', function () {
            logLines = [];
            unshown = 0;
            logView.textContent = '';
          })
        ])),
        logView,
        element('div', { 'class': 'vc1-row' }, [input, button('Run', execute)]),
        element('p', { 'class': 'vc1-note', text: 'The game’s log, and Squirrel lines run in the game as the engine’s console would (the game never opens that one).' })
      ]
    };
  }

  // ---- Mods tab ------------------------------------------------------------

  // Texture mods (src/mod/): MODS/<id>/mod.json and its PNGs; enabled.txt
  // lists the active ones in load order. The game reads them at start.
  function modFolders() {
    if (!exists(MODS)) {
      return [];
    }
    return FS.readdir(MODS).filter(function (name) {
      return name !== '.' && name !== '..' && FS.isDir(FS.stat(MODS + '/' + name).mode);
    });
  }

  function readEnabled() {
    if (!exists(MODS + '/enabled.txt')) {
      return [];
    }
    return FS.readFile(MODS + '/enabled.txt', { encoding: 'utf8' }).split(/\r?\n/).map(function (line) {
      return line.trim();
    }).filter(function (line) {
      return line && line[0] !== '#';
    });
  }

  function writeEnabled(names) {
    makeTree(MODS);
    FS.writeFile(MODS + '/enabled.txt', '# One mod directory per line. Later entries override earlier entries.\n' + names.join('\n') + '\n');
  }

  function manifest(name) {
    try {
      return JSON.parse(FS.readFile(MODS + '/' + name + '/mod.json', { encoding: 'utf8' }));
    } catch (error) {
      return null;
    }
  }

  // Installs files ({path, bytes}) holding one mod.json: its folder becomes
  // MODS/<id>.
  function installMod(files) {
    var json = files.filter(function (file) {
      return /(^|\/)mod\.json$/.test(file.path);
    }).sort(function (a, b) {
      return a.path.length - b.path.length;
    })[0];
    if (!json) {
      throw new Error('no mod.json in what was picked');
    }
    var info = JSON.parse(new TextDecoder().decode(json.bytes));
    if (!info.id || /[\/\\:]/.test(info.id) || info.id === '.' || info.id === '..') {
      throw new Error('mod.json needs a plain "id"');
    }
    var base = json.path.slice(0, json.path.length - 'mod.json'.length);
    var target = MODS + '/' + info.id;
    if (exists(target)) {
      removeTree(target);
    }
    files.forEach(function (file) {
      if (file.path.slice(0, base.length) !== base || /\/$/.test(file.path)) {
        return;
      }
      var path = target + '/' + file.path.slice(base.length);
      makeTree(path.slice(0, path.lastIndexOf('/')));
      FS.writeFile(path, file.bytes);
    });
    var enabled = readEnabled();
    if (enabled.indexOf(info.id) < 0) {
      enabled.push(info.id);
      writeEnabled(enabled);
    }
    return info.id;
  }

  // The entries of a zip archive (stored or deflated).
  function unzip(buffer) {
    var view = new DataView(buffer);
    var end = -1;
    for (var at = buffer.byteLength - 22; at >= Math.max(0, buffer.byteLength - 65558); at--) {
      if (view.getUint32(at, true) === 0x06054b50) {
        end = at;
        break;
      }
    }
    if (end < 0) {
      return Promise.reject(new Error('not a zip archive'));
    }
    var count = view.getUint16(end + 10, true);
    var entry = view.getUint32(end + 16, true);
    var reads = [];
    for (var i = 0; i < count; i++) {
      var method = view.getUint16(entry + 10, true);
      var size = view.getUint32(entry + 20, true);
      var nameLength = view.getUint16(entry + 28, true);
      var extraLength = view.getUint16(entry + 30, true);
      var commentLength = view.getUint16(entry + 32, true);
      var local = view.getUint32(entry + 42, true);
      var path = new TextDecoder().decode(new Uint8Array(buffer, entry + 46, nameLength)).replace(/\\/g, '/');
      var start = local + 30 + view.getUint16(local + 26, true) + view.getUint16(local + 28, true);
      reads.push(inflate(new Uint8Array(buffer, start, size), method).then(function (path, bytes) {
        return { path: path, bytes: bytes };
      }.bind(null, path)));
      entry += 46 + nameLength + extraLength + commentLength;
    }
    return Promise.all(reads);
  }

  function inflate(bytes, method) {
    if (method === 0) {
      return Promise.resolve(bytes.slice());
    }
    if (method !== 8) {
      return Promise.reject(new Error('unsupported zip compression ' + method));
    }
    var stream = new Blob([bytes]).stream().pipeThrough(new DecompressionStream('deflate-raw'));
    return new Response(stream).arrayBuffer().then(function (result) {
      return new Uint8Array(result);
    });
  }

  function modsTab() {
    var list = element('div');

    function refresh() {
      list.textContent = '';
      var folders = modFolders();
      var enabled = readEnabled().filter(function (name) {
        return folders.indexOf(name) >= 0;
      });
      var order = enabled.concat(folders.filter(function (name) {
        return enabled.indexOf(name) < 0;
      }));
      if (!order.length) {
        list.appendChild(element('p', { 'class': 'vc1-note', text: 'No mods installed.' }));
      }
      order.forEach(function (name) {
        var info = manifest(name) || {};
        var on = enabled.indexOf(name) >= 0;
        var check = element('input', { type: 'checkbox' });
        check.checked = on;
        check.addEventListener('change', function () {
          writeEnabled(check.checked ? enabled.concat([name]) : enabled.filter(function (other) {
            return other !== name;
          }));
          refresh();
        });
        function move(step) {
          var index = enabled.indexOf(name);
          var next = enabled.slice();
          next.splice(index, 1);
          next.splice(Math.max(0, Math.min(next.length, index + step)), 0, name);
          writeEnabled(next);
          refresh();
        }
        list.appendChild(element('div', { 'class': 'vc1-row vc1-file' }, [
          check,
          element('span', { 'class': 'vc1-grow', text: name + (info.version ? ' ' + info.version : '') + ' – ' + Object.keys(info.tiles || {}).length + ' tiles' }),
          on ? button('↑', function () {
            move(-1);
          }, 'Load earlier') : null,
          on ? button('↓', function () {
            move(1);
          }, 'Load later (wins over earlier mods)') : null,
          button('Delete', function () {
            if (confirm('Delete the mod ' + name + '?')) {
              removeTree(MODS + '/' + name);
              writeEnabled(enabled.filter(function (other) {
                return other !== name;
              }));
              refresh();
            }
          })
        ]));
      });
    }

    function installed(id) {
      say('Installed ' + id + '; reload to apply');
      refresh();
    }

    return {
      name: 'Mods',
      shown: refresh,
      body: [
        list,
        element('div', { 'class': 'vc1-row' }, [
          button('Add a mod folder', function () {
            pickFiles({ folder: true }, function (files) {
              Promise.all(files.map(function (file) {
                return file.arrayBuffer().then(function (buffer) {
                  return { path: file.webkitRelativePath || file.name, bytes: new Uint8Array(buffer) };
                });
              })).then(installMod).then(installed).catch(failed);
            });
          }),
          button('Add a mod .zip', function () {
            pickFiles({ accept: '.zip' }, function (files) {
              files[0].arrayBuffer().then(unzip).then(installMod).then(installed).catch(failed);
            });
          }),
          button('Reload to apply', saveAndReload)
        ]),
        element('p', { 'class': 'vc1-note', text: 'A mod is a folder with mod.json ({"id", "version", "tiles": {tile: "file.png"}}) and its PNGs; later mods win. The game loads them when it starts.' })
      ]
    };
  }

  // ---- The panel -----------------------------------------------------------

  var panel;
  var tabs;
  var current = null;

  function build() {
    tabs = [gameTab(), savesTab(), cheatsTab(), consoleTab(), modsTab()];
    var bar = element('nav', { 'class': 'vc1-tabs' });
    var pages = element('div', { 'class': 'vc1-pages' });
    tabs.forEach(function (tab) {
      tab.button = button(tab.name, function () {
        select(tab);
      });
      tab.page = element('section', { hidden: '' }, tab.body);
      bar.appendChild(tab.button);
      pages.appendChild(tab.page);
    });
    bar.appendChild(button('×', toggle, 'Close (F10)'));
    panel = element('aside', { 'class': 'vc1-panel', hidden: '' }, [bar, pages, status]);
    // Typing in the panel is not for the game: SDL listens on the window.
    ['keydown', 'keyup', 'keypress'].forEach(function (type) {
      panel.addEventListener(type, function (event) {
        if (event.key !== 'F10') {
          event.stopPropagation();
        }
      });
    });
    document.body.appendChild(panel);
    select(tabs[0]);
  }

  function select(tab) {
    tabs.forEach(function (other) {
      other.page.hidden = other !== tab;
      other.button.classList.toggle('vc1-on', other === tab);
    });
    current = tab;
    if (tab.shown && !panel.hidden) {
      tab.shown();
    }
  }

  function toggle() {
    if (!panel) {
      build();
    }
    panel.hidden = !panel.hidden;
    if (panel.hidden) {
      document.getElementById('canvas').focus();
    } else if (current.shown) {
      current.shown();
    }
  }

  window.addEventListener('keydown', function (event) {
    if (event.key === 'F10') {
      event.preventDefault();
      event.stopImmediatePropagation();
      toggle();
    }
  }, true);

  // index.html loads this script in the body.
  document.body.appendChild(button('⚙', toggle, 'Control panel (F10)')).className = 'vc1-open';
})();
