/*
 * libiui WebAssembly JavaScript Glue Code
 *
 * Provides Canvas 2D rendering and event handling for the WASM backend.
 * Pattern inspired by Mado's mado-wasm.js.
 */

/* Canvas rendering module */
const IuiCanvas = {
    canvas: null,
    ctx: null,
    dpr: 1, /* Device pixel ratio */

    getContext: function () {
        return this.ctx;
    },
    parseColor: function (c) {
        const a = (c >> 24) & 0xFF;
        const r = (c >> 16) & 0xFF;
        const g = (c >> 8) & 0xFF;
        const b = c & 0xFF;
        return `rgba(${r},${g},${b},${a / 255})`;
    },

    /* Initialize Canvas context */
    init: function (width, height) {
        console.log("[IuiCanvas] init() called with " + width + "x" + height);
        if (typeof addConsoleLine === "function") {
            addConsoleLine("Canvas init: " + width + "x" + height, false);
        }

        this.canvas = document.getElementById("iui-canvas");
        if (!this.canvas) {
            /* Create canvas if not found */
            this.canvas = document.createElement("canvas");
            this.canvas.id = "iui-canvas";
            document.body.appendChild(this.canvas);
        }

        this.width = width;
        this.height = height;
        this.dpr = window.devicePixelRatio || 1;

        /* Set canvas size (CSS pixels) */
        this.canvas.style.width = width + "px";
        this.canvas.style.height = height + "px";

        /* Set actual canvas size (physical pixels for HiDPI) */
        this.canvas.width = width * this.dpr;
        this.canvas.height = height * this.dpr;

        this.ctx = this.canvas.getContext("2d");
        this.ctx.scale(this.dpr, this.dpr);

        /* Create ImageData for framebuffer transfer (logical size) */


        /* Setup event listeners */
        this.setupEvents();

        console.log("[IuiCanvas] Canvas initialized: " + width + "x" + height + " (DPR: " + this.dpr + ")");
        return true;
    },

    /* Setup event listeners */
    setupEvents: function () {
        var canvas = this.canvas;
        var self = this;

        /* Mouse motion - get fresh bounding rect each event for scroll/resize handling */
        canvas.addEventListener("mousemove", function (e) {
            var rect = canvas.getBoundingClientRect();
            var x = Math.floor(e.clientX - rect.left);
            var y = Math.floor(e.clientY - rect.top);
            var buttons = e.buttons;
            Module._iui_wasm_mouse_motion(x, y, buttons);
        });

        /* Mouse button down */
        canvas.addEventListener("mousedown", function (e) {
            e.preventDefault();
            var rect = canvas.getBoundingClientRect();
            var x = Math.floor(e.clientX - rect.left);
            var y = Math.floor(e.clientY - rect.top);
            Module._iui_wasm_mouse_button(x, y, e.button, 1);
        });

        /* Mouse button up */
        canvas.addEventListener("mouseup", function (e) {
            e.preventDefault();
            var rect = canvas.getBoundingClientRect();
            var x = Math.floor(e.clientX - rect.left);
            var y = Math.floor(e.clientY - rect.top);
            Module._iui_wasm_mouse_button(x, y, e.button, 0);
        });

        /* Mouse wheel */
        canvas.addEventListener("wheel", function (e) {
            Module._iui_wasm_scroll(e.deltaX, e.deltaY);
            e.preventDefault();
        }, { passive: false });

        /* Keyboard events */
        document.addEventListener("keydown", function (e) {
            var key = self.mapKey(e.keyCode, e.key);
            if (key !== 0) {
                Module._iui_wasm_key(key, 1, e.shiftKey ? 1 : 0);
                /* Prevent default for navigation keys */
                if (key >= 1 && key <= 12) e.preventDefault();
            }
        });

        document.addEventListener("keyup", function (e) {
            var key = self.mapKey(e.keyCode, e.key);
            if (key !== 0) {
                Module._iui_wasm_key(key, 0, e.shiftKey ? 1 : 0);
            }
        });

        /* Text input (keypress for printable characters) */
        document.addEventListener("keypress", function (e) {
            if (e.key.length === 1 && !e.ctrlKey && !e.metaKey) {
                Module._iui_wasm_char(e.key.charCodeAt(0));
            }
        });

        /* Prevent context menu on canvas */
        canvas.addEventListener("contextmenu", function (e) {
            e.preventDefault();
        });

        console.log("[libiui] Event handlers initialized");
    },

    /* Map browser keyCode to iui_key_code enum */
    mapKey: function (keyCode, key) {
        switch (keyCode) {
            case 8: return 1;   /* Backspace -> key_backspace */
            case 46: return 2;  /* Delete -> key_delete */
            case 37: return 3;  /* Left -> key_left */
            case 39: return 4;  /* Right -> key_right */
            case 36: return 5;  /* Home -> key_home */
            case 35: return 6;  /* End -> key_end */
            case 13: return 7;  /* Enter -> key_enter */
            case 9: return 8;   /* Tab -> key_tab */
            case 27: return 9;  /* Escape -> key_escape */
            case 38: return 10; /* Up -> key_up */
            case 40: return 11; /* Down -> key_down */
            case 32: return 12; /* Space -> key_space */
            default: return 0;
        }
    },



    /* Cleanup */
    destroy: function () {
        this.canvas = null;
        this.ctx = null;
        this.imageData = null;
    }
};

/* Emscripten Module configuration */
var Module = Module || {};

/* Tell Emscripten where to find the WASM file */
Module.locateFile = function (path, prefix) {
    console.log("[libiui] locateFile: " + path + " (prefix: " + prefix + ")");
    /* Use provided prefix to support different deployment paths */
    return prefix + path;
};

/* Error handling */
Module.onAbort = function (what) {
    console.error("[libiui] ABORT: " + what);
    if (typeof addConsoleLine === "function") {
        addConsoleLine("ABORT: " + what, true);
    }
};

/* Called when WebAssembly module is ready */
Module.onRuntimeInitialized = function () {
    console.log("[libiui] WebAssembly runtime initialized");

    /* Event handlers are initialized in IuiCanvas.init() called from C code */
    /* Main loop is started by C code via emscripten_set_main_loop_arg */
};

/* Monitor loading progress */
Module.monitorRunDependencies = function (left) {
    console.log("[libiui] Run dependencies remaining: " + left);
};

/* Print output to console */
Module.print = function (text) {
    console.log("[C stdout] " + text);
    if (typeof addConsoleLine === "function") {
        addConsoleLine("[C] " + text, false);
    }
};

/* Print errors to console */
Module.printErr = function (text) {
    console.error("[C stderr] " + text);
    if (typeof addConsoleLine === "function") {
        addConsoleLine("[C err] " + text, true);
    }
};

/* Intercept ALL console.log to capture EM_ASM output */
(function() {
    var originalLog = console.log;
    var originalError = console.error;
    console.log = function() {
        originalLog.apply(console, arguments);
        /* Forward important messages to on-page console */
        var msg = Array.prototype.slice.call(arguments).join(" ");
        if (msg.indexOf("[example]") >= 0 || msg.indexOf("[libiui]") >= 0 || msg.indexOf("[IuiCanvas]") >= 0) {
            if (typeof addConsoleLine === "function") {
                addConsoleLine(msg, false);
            }
        }
    };
    console.error = function() {
        originalError.apply(console, arguments);
        var msg = Array.prototype.slice.call(arguments).join(" ");
        if (typeof addConsoleLine === "function") {
            addConsoleLine("ERROR: " + msg, true);
        }
    };
})();

/* Canvas element reference */
Module.canvas = (function () {
    return document.getElementById("iui-canvas");
})();

/* Global error handler */
window.onerror = function (msg, url, line, col, error) {
    console.error("[iui-wasm.js] ERROR: " + msg + " at " + url + ":" + line + ":" + col);
    if (typeof addConsoleLine === "function") {
        addConsoleLine("ERROR: " + msg, true);
    }
    return false;
};

/* Handle promise rejections */
window.onunhandledrejection = function (event) {
    console.error("[iui-wasm.js] Unhandled rejection: " + event.reason);
    if (typeof addConsoleLine === "function") {
        addConsoleLine("Unhandled rejection: " + event.reason, true);
    }
};

console.log("[iui-wasm.js] Module configured, waiting for libiui_example.js...");
if (typeof addConsoleLine === "function") {
    addConsoleLine("Module configured, loading WASM...", false);
}
