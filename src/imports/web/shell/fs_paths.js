// Linked with --post-js (cmake/web.cmake). The recovered code and the SDL3
// imports pass Windows paths to the CRT: backslashes as separators
// (PathAppendA) and names in any case, as NTFS allows. Every path a syscall
// receives is resolved here: separators become '/', and a name that does not
// exist is matched case-insensitively against its directory.
(function () {
  function exists(path) {
    try {
      FS.lookupPath(path, { follow: true });
      return true;
    } catch (e) {
      return false;
    }
  }

  function resolve(path) {
    if (path.indexOf('\\') >= 0) {
      path = path.replace(/\\/g, '/');
    }
    if (!path.length || exists(path)) {
      return path;
    }
    var cwd = FS.cwd();
    var absolute = path.charAt(0) === '/' ? path : (cwd === '/' ? '' : cwd) + '/' + path;
    var parts = absolute.split('/');
    var out = '';
    for (var i = 0; i < parts.length; ++i) {
      var part = parts[i];
      if (!part.length || part === '.') {
        continue;
      }
      if (part === '..') {
        out = out.substring(0, out.lastIndexOf('/'));
        continue;
      }
      var next = out + '/' + part;
      if (!exists(next)) {
        var names;
        try {
          names = FS.readdir(out || '/');
        } catch (e) {
          // The rest does not exist; keep it for a create.
          return out + '/' + parts.slice(i).join('/');
        }
        var lower = part.toLowerCase();
        for (var j = 0; j < names.length; ++j) {
          if (names[j].toLowerCase() === lower) {
            next = out + '/' + names[j];
            break;
          }
        }
      }
      out = next;
    }
    return out || '/';
  }

  var getStr = SYSCALLS.getStr;
  SYSCALLS.getStr = function (ptr) {
    return resolve(getStr(ptr));
  };
})();
