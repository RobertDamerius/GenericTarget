/* ==========================================================================
   Generic Target Documentation - Theme Init
   Loaded in <head> (synchronously, before first paint) so the saved theme
   preference is applied before the page renders — no flash of the wrong
   theme. The full theme toggle logic lives in assets/script.js.

   Priority: theme in the URL (the global setting — every link of the site
   carries ?theme=..., which works even though Firefox isolates localStorage
   per file:// page) -> the tab's choice (window.name) -> this page's stored
   choice -> OS preference.
   ========================================================================== */
try {
  var _t = null;
  try {
    var _q = (location.search || "").replace(/^\?/, "");
    var _parts = _q ? _q.split("&") : [];
    for (var _i = 0; _i < _parts.length; _i++) {
      if (_parts[_i] === "theme=dark" || _parts[_i] === "theme=light") {
        _t = _parts[_i].slice(6);
        break;
      }
    }
  } catch (e) {}
  if (!_t) {
    var _m = (window.name || "").match(/gtTheme=(dark|light)/);
    if (_m) { _t = _m[1]; }
  }
  if (!_t) {
    var _s = localStorage.getItem("gt-docs-theme");
    if (_s === "light" || _s === "dark") { _t = _s; }
  }
  if (_t) { document.documentElement.setAttribute("data-theme", _t); }
} catch (e) {}
