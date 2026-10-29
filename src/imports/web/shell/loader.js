// The web build's page logic. game.data.js (cmake/web.cmake) preloads the
// local VirtualCity.bar and the original cursors at /game; saves
// (%APPDATA%) persist in IndexedDB. Then main (WinMain) runs.
'use strict';

var GAME = '/game';  // the working directory: VirtualCity.bar
var SAVE = '/save';  // %APPDATA%: profiles and settings

// F12 and the devtools shortcuts belong to the browser: SDL would take them
// as game keys and cancel them. This capture listener runs before SDL's.
function passBrowserKeys(event) {
  if (event.key === 'F12' || (event.ctrlKey && event.shiftKey && /^[ijc]$/i.test(event.key))) {
    event.stopImmediatePropagation();
  }
}
window.addEventListener('keydown', passBrowserKeys, true);
window.addEventListener('keyup', passBrowserKeys, true);

function saveSync() {
  FS.syncfs(false, function () {});
}

var Module = {
  canvas: document.getElementById('canvas'),
  preRun: [function () {
    FS.mkdir(SAVE);
    FS.mount(IDBFS, {}, SAVE);
    ENV.APPDATA = SAVE;
    Module.addRunDependency('vc1-saves');
    FS.syncfs(true, function (error) {
      if (error) {
        console.error('IndexedDB', error);
      }
      Module.removeRunDependency('vc1-saves');
    });
  }],
  onRuntimeInitialized: function () {
    // user32.cpp loads cursors from <base path>/platform-resources.
    FS.symlink(GAME + '/.vc1/platform-resources', '/platform-resources');
    FS.chdir(GAME);
    // Saves are written as the game goes; keep IndexedDB current.
    setInterval(saveSync, 5000);
    document.getElementById('canvas').focus();
    // main (WinMain) runs next.
  },
  onExit: saveSync,
  print: function (text) {
    console.log(text);
  },
  printErr: function (text) {
    console.warn(text);
  }
};
