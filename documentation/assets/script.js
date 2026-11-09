/* ==========================================================================
   Generic Target Documentation - Local Enhancements
   All functionality is progressive enhancement: the documentation is fully
   readable and navigable with JavaScript disabled.
   No external libraries or network access.
   ========================================================================== */
(function () {
  "use strict";

  /* ------------------------------------------------------------------
   * Theme toggle (global across the whole documentation)
   *
   * Firefox isolates localStorage per file:// page, so per-page storage
   * cannot hold a shared setting. The theme is therefore the global
   * setting carried in the URL (?theme=dark|light): it is attached to
   * every internal link, so one toggle applies to all pages, tabs,
   * bookmarks, and history entries. localStorage and window.name are kept
   * only as fallbacks for URLs that do not carry the parameter yet
   * (e.g. pages opened directly).
   * ------------------------------------------------------------------ */
  var THEME_KEY = "gt-docs-theme";

  function applyTheme(theme) {
    if (theme === "light" || theme === "dark") {
      document.documentElement.setAttribute("data-theme", theme);
    } else {
      document.documentElement.removeAttribute("data-theme");
    }
  }

  // The effective theme: the explicit choice if one exists, otherwise the
  // OS preference.
  function currentTheme() {
    var theme = document.documentElement.getAttribute("data-theme");
    if (theme === "light" || theme === "dark") {
      return theme;
    }
    var prefersDark = window.matchMedia &&
      window.matchMedia("(prefers-color-scheme: dark)").matches;
    return prefersDark ? "dark" : "light";
  }

  // href with ?theme=... attached (an existing theme parameter is replaced,
  // other query parameters and the fragment are preserved).
  function withThemeParam(href, theme) {
    var fragment = "";
    var hashIdx = href.indexOf("#");
    if (hashIdx >= 0) {
      fragment = href.slice(hashIdx);
      href = href.slice(0, hashIdx);
    }
    var query = "";
    var qIdx = href.indexOf("?");
    if (qIdx >= 0) {
      query = href.slice(qIdx + 1);
      href = href.slice(0, qIdx);
    }
    var parts = [];
    if (query) {
      Array.prototype.forEach.call(query.split("&"), function (part) {
        if (part && part.indexOf("theme=") !== 0) {
          parts.push(part);
        }
      });
    }
    parts.push("theme=" + theme);
    return href + "?" + parts.join("&") + fragment;
  }

  // Internal link: relative page link without fragment-only navigation.
  function isInternalHref(href) {
    var h = href.trim();
    if (!h || h.charAt(0) === "#") {
      return false;
    }
    var lower = h.toLowerCase();
    if (lower.indexOf("//") === 0) {
      return false;
    }
    return !/^(https?:|mailto:|tel:|javascript:|data:|ftp:|ws:|wss:)/
      .test(lower);
  }

  // Attach the theme to every internal link so the choice travels with
  // every navigation — all pages, tabs, bookmarks, and history.
  function propagateThemeToLinks(theme) {
    var links = document.querySelectorAll("a[href]");
    Array.prototype.forEach.call(links, function (a) {
      var href = a.getAttribute("href");
      if (href && isInternalHref(href)) {
        a.setAttribute("href", withThemeParam(href, theme));
      }
    });
  }

  // Put the theme into the current URL (same-document update, no reload)
  // so it survives reloads, bookmarks, and "open in new tab".
  function setUrlTheme(theme) {
    try {
      var parts = [];
      if (location.search) {
        Array.prototype.forEach.call(
          location.search.slice(1).split("&"), function (part) {
            if (part && part.indexOf("theme=") !== 0) {
              parts.push(part);
            }
          });
      }
      parts.push("theme=" + theme);
      var url = location.pathname + "?" + parts.join("&") + location.hash;
      if (location.href !== url) {
        window.history.replaceState(null, "", url);
      }
    } catch (err) {
      /* replaceState unavailable; links still carry the theme */
    }
  }

  // Fallback stores (per tab / per page) for URLs without the parameter.
  function storeTheme(theme) {
    try {
      window.localStorage.setItem(THEME_KEY, theme);
    } catch (err) {
      /* localStorage unavailable (e.g. blocked); theme still applies */
    }
    try {
      window.name = "gtTheme=" + theme;
    } catch (err) {
      /* window.name unavailable; theme still applies on this page */
    }
  }

  function initThemeToggle() {
    var buttons = document.querySelectorAll("[data-theme-toggle]");
    Array.prototype.forEach.call(buttons, function (button) {
      button.addEventListener("click", function () {
        var next = currentTheme() === "dark" ? "light" : "dark";
        applyTheme(next);
        storeTheme(next);
        setUrlTheme(next);
        propagateThemeToLinks(next);
        updateToggleLabels();
      });
    });
  }

  function updateToggleLabels() {
    var theme = currentTheme();
    var buttons = document.querySelectorAll("[data-theme-toggle]");
    Array.prototype.forEach.call(buttons, function (button) {
      var label = button.querySelector("[data-theme-label]");
      if (label) {
        label.textContent = theme === "dark" ? "Light mode" : "Dark mode";
      }
      button.setAttribute("aria-label",
        theme === "dark" ? "Switch to light mode" : "Switch to dark mode");
    });
  }

  /* ------------------------------------------------------------------
   * Sidebar scroll position handoff
   *
   * The sidebar (nav.site-nav) scrolls independently of the page, and
   * moving to another page resets it to the top. The current offset is
   * therefore carried over to the next page. Like the theme, the value
   * travels as a query parameter (?navScroll=<px>) attached to every
   * internal link — that works even though Firefox isolates Web Storage
   * per file:// page. sessionStorage is kept as a fallback for
   * navigations that do not fire a click (e.g. "Open link in new tab").
   * ------------------------------------------------------------------ */
  var NAV_SCROLL_PARAM = "navScroll";
  var NAV_SCROLL_KEY = "gt-docs-nav-scroll";

  function navScrollEl() {
    return document.querySelector("nav.site-nav");
  }

  function currentNavScroll() {
    var nav = navScrollEl();
    return nav ? Math.round(nav.scrollTop) : 0;
  }

  // href with ?navScroll=<px> attached (an existing navScroll parameter is
  // replaced, other query parameters and the fragment are preserved).
  function withNavScrollParam(href, value) {
    var fragment = "";
    var hashIdx = href.indexOf("#");
    if (hashIdx >= 0) {
      fragment = href.slice(hashIdx);
      href = href.slice(0, hashIdx);
    }
    var query = "";
    var qIdx = href.indexOf("?");
    if (qIdx >= 0) {
      query = href.slice(qIdx + 1);
      href = href.slice(0, qIdx);
    }
    var parts = [];
    if (query) {
      Array.prototype.forEach.call(query.split("&"), function (part) {
        if (part && part.indexOf(NAV_SCROLL_PARAM + "=") !== 0) {
          parts.push(part);
        }
      });
    }
    parts.push(NAV_SCROLL_PARAM + "=" + value);
    return href + "?" + parts.join("&") + fragment;
  }

  // Attach the offset to every internal link so it travels with any
  // navigation — all pages, tabs, bookmarks, and history.
  function propagateNavScrollToLinks(value) {
    var links = document.querySelectorAll("a[href]");
    Array.prototype.forEach.call(links, function (a) {
      var href = a.getAttribute("href");
      if (href && isInternalHref(href)) {
        a.setAttribute("href", withNavScrollParam(href, value));
      }
    });
  }

  function storeNavScroll(value) {
    try {
      window.sessionStorage.setItem(NAV_SCROLL_KEY, String(value));
    } catch (err) {
      /* sessionStorage unavailable (e.g. blocked); links still carry it */
    }
  }

  function readStoredNavScroll() {
    try {
      var v = window.sessionStorage.getItem(NAV_SCROLL_KEY);
      if (v !== null && /^\d+$/.test(v)) {
        return parseInt(v, 10);
      }
    } catch (err) {
      /* sessionStorage unavailable; the URL parameter still applies */
    }
    return null;
  }

  function navScrollFromUrl() {
    var m = (location.search || "").match(/[?&]navScroll=(\d+)/);
    return m ? parseInt(m[1], 10) : null;
  }

  // Restore the offset carried in the URL (or the fallback store) as
  // soon as the sidebar is laid out. The sidebar is a plain text list
  // (no images), so its height is stable; the requestAnimationFrame pass
  // only guards against late layout changes. Values beyond the current
  // scroll range are clamped by the browser automatically.
  function restoreNavScroll(value) {
    if (value === null) {
      return;
    }
    storeNavScroll(value);
    var apply = function () {
      var nav = navScrollEl();
      if (nav) {
        nav.scrollTop = value;
      }
    };
    apply();
    if (window.requestAnimationFrame) {
      window.requestAnimationFrame(apply);
    }
  }

  // Closest <a> at or above the element (conservative; no closest()).
  function closestAnchor(el) {
    while (el && el.nodeType === 1) {
      if (el.tagName === "A") {
        return el;
      }
      el = el.parentElement;
    }
    return null;
  }

  // Keep every link (and the fallback store) up to date while the user
  // scrolls the sidebar, so also non-click navigations carry the current
  // offset. Debounced so it runs only once the scrolling settles.
  var navScrollTimer = null;
  function scheduleNavScrollSync() {
    if (navScrollTimer !== null) {
      window.clearTimeout(navScrollTimer);
    }
    navScrollTimer = window.setTimeout(function () {
      navScrollTimer = null;
      var value = currentNavScroll();
      propagateNavScrollToLinks(value);
      storeNavScroll(value);
    }, 120);
  }

  function initNavScroll() {
    // The URL parameter is the reliable channel (it survives file://
    // storage isolation); the fallback store covers links opened without
    // a click in browsers that share storage.
    var value = navScrollFromUrl();
    if (value === null) {
      value = readStoredNavScroll();
    }
    if (value !== null) {
      restoreNavScroll(value);
      // Attach the inherited value right away, so links carry it even
      // before the first scroll.
      propagateNavScrollToLinks(value);
    }

    var nav = navScrollEl();
    if (nav) {
      nav.addEventListener("scroll", scheduleNavScrollSync, { passive: true });
    }

    // On every internal link click (plain, middle, Ctrl/Cmd), attach the
    // exact current offset to the clicked link before navigation starts.
    document.addEventListener("click", function (event) {
      var a = closestAnchor(event.target);
      if (!a) {
        return;
      }
      var href = a.getAttribute("href");
      if (!href || !isInternalHref(href)) {
        return;
      }
      var scroll = currentNavScroll();
      storeNavScroll(scroll);
      a.setAttribute("href", withNavScrollParam(href, scroll));
    });
  }

  /* ------------------------------------------------------------------
   * Copy-to-clipboard buttons for code blocks
   * ------------------------------------------------------------------ */
  function copyText(text) {
    if (navigator.clipboard && navigator.clipboard.writeText) {
      // Note: writeText() resolves with undefined, so map success to true
      // explicitly; otherwise a successful copy is reported as a failure.
      return navigator.clipboard.writeText(text).then(function () {
        return true;
      }).catch(function () {
        return fallbackCopy(text);
      });
    }
    return Promise.resolve(fallbackCopy(text));
  }

  function fallbackCopy(text) {
    var textarea = document.createElement("textarea");
    textarea.value = text;
    textarea.setAttribute("readonly", "");
    textarea.style.position = "fixed";
    textarea.style.left = "-9999px";
    document.body.appendChild(textarea);
    textarea.select();
    var ok = false;
    try {
      ok = document.execCommand("copy");
    } catch (err) {
      ok = false;
    }
    document.body.removeChild(textarea);
    return Promise.resolve(ok);
  }

  function initCopyButtons() {
    var pres = document.querySelectorAll("pre");
    Array.prototype.forEach.call(pres, function (pre) {
      if (pre.parentElement && pre.parentElement.classList.contains("codeblock")) {
        return; // already wrapped
      }
      var wrapper = document.createElement("div");
      wrapper.className = "codeblock";
      pre.parentNode.insertBefore(wrapper, pre);
      wrapper.appendChild(pre);

      var button = document.createElement("button");
      button.type = "button";
      button.className = "copy-btn";
      button.textContent = "Copy";
      button.setAttribute("aria-label", "Copy code to clipboard");
      button.addEventListener("click", function () {
        var codeEl = pre.querySelector("code");
        var text = (codeEl || pre).textContent;
        copyText(text).then(function (ok) {
          button.textContent = ok ? "Copied" : "Copy failed";
          button.classList.add("copied");
          window.clearTimeout(button._resetTimer);
          button._resetTimer = window.setTimeout(function () {
            button.textContent = "Copy";
            button.classList.remove("copied");
          }, 2000);
        });
      });
      wrapper.appendChild(button);
    });
  }

  /* ------------------------------------------------------------------
   * Back to top
   * ------------------------------------------------------------------ */
  function initBackToTop() {
    var button = document.getElementById("backToTop");
    if (!button) {
      return;
    }
    var onScroll = function () {
      var scrolled = window.scrollY || document.documentElement.scrollTop;
      if (scrolled > 400) {
        button.classList.add("visible");
      } else {
        button.classList.remove("visible");
      }
    };
    window.addEventListener("scroll", onScroll, { passive: true });
    onScroll();
    button.addEventListener("click", function () {
      window.scrollTo({ top: 0, behavior: "smooth" });
    });
  }

  /* ------------------------------------------------------------------
   * Jump to the article on fresh page loads (narrow windows only)
   *
   * In the narrow layout (the max-width media query in design.css turns
   * .layout into a column) the whole navigation renders *above* the
   * article, and a fresh page load therefore starts at the top of the
   * navigation menu. On fresh navigations (link click, first load,
   * address bar) jump straight to the beginning of the content, so the
   * user does not have to scroll past the entire menu first.
   *
   * Deliberately skipped:
   *  - back/forward and reload: the browser restores the previous
   *    scroll position, which is kept;
   *  - URLs with a fragment (#section): the browser's own fragment
   *    scroll is respected;
   *  - wide windows: the article already starts at the top.
   *
   * This script is the last element of <body>, so the DOM is complete
   * here and the jump can happen before the first paint — no visible
   * flicker of the navigation on top.
   * ------------------------------------------------------------------ */
  function scrollToContentOnLoad() {
    try {
      if (location.hash) {
        return; // let the browser's fragment navigation take over
      }
      var type = "navigate";
      if (window.performance && performance.getEntriesByType) {
        var entries = performance.getEntriesByType("navigation");
        if (entries.length) {
          type = entries[0].type;
        }
      }
      if (type !== "navigate" && type !== "prerender") {
        return; // keep the scroll position restored by the browser
      }
      var layout = document.querySelector(".layout");
      if (!layout || !window.getComputedStyle) {
        return;
      }
      // Only the stacked layout puts the sidebar above the article
      // (column direction is set by the narrow-window media query).
      if (getComputedStyle(layout).flexDirection !== "column") {
        return;
      }
      var content = document.querySelector(".content-main") ||
        document.getElementById("main-content");
      if (!content || !content.getBoundingClientRect) {
        return;
      }
      // Keep the top of the content below the sticky header.
      var header = document.querySelector(".site-header");
      var headerHeight = header ? header.offsetHeight : 0;
      var target = content.getBoundingClientRect().top +
        (window.pageYOffset || window.scrollY || 0) - headerHeight;
      if (target <= 0) {
        return;
      }
      // The stylesheet sets scroll-behavior: smooth on <html>; override
      // it temporarily so the view jumps directly instead of animating.
      var root = document.documentElement;
      var previous = root.style.scrollBehavior;
      root.style.scrollBehavior = "auto";
      try {
        window.scrollTo(0, target);
      } finally {
        root.style.scrollBehavior = previous;
      }
    } catch (err) {
      /* progressive enhancement: any failure keeps the default behavior */
    }
  }

  // Runs synchronously at parse time (see note above) so the jump
  // happens before the first paint whenever possible.
  scrollToContentOnLoad();

  function ready(fn) {
    if (document.readyState === "loading") {
      document.addEventListener("DOMContentLoaded", fn);
    } else {
      fn();
    }
  }

  ready(function () {
    initThemeToggle();
    updateToggleLabels();
    // An explicit theme choice (from the URL, the tab, or this page's
    // storage) becomes global right away: put it into the current URL and
    // every internal link, even on pages opened directly.
    var explicit = document.documentElement.getAttribute("data-theme");
    if (explicit === "light" || explicit === "dark") {
      setUrlTheme(explicit);
      propagateThemeToLinks(explicit);
    }
    initCopyButtons();
    initBackToTop();
    initNavScroll();
  });
})();
