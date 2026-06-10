---
applyTo: "wled00/data/**"
---
# Web UI Coding Conventions

## Formatting

- Indent **HTML and JavaScript** with **tabs**
- Indent **CSS** with **tabs** or **spaces**

## JavaScript Style

- **camelCase** for functions and variables: `gId()`, `selectedFx`, `currentPreset`
- Abbreviated helpers are common: `d` for `document`, `gId()` for `getElementById()`
- Mark WLED-MM additions with `// WLEDMM` comments

## Key Files

- `index.htm` — main interface
- `index.js` — functions that manage / update the main interface
- peek.js, liveview*.htm - live preview in main interface
- `settings*.htm` — configuration pages
- `*.css` — stylesheets (inlined during build)

## Accessibility & Interaction

The WLED web UI targets commonly used browser/platform combinations: desktop browsers on Mac and PC (primarily pointer-driven, touch rare), 
and touch-only devices (phones, tablets). If possible, keep the UI accessible to users with disabilities. 
Full keyboard operability is not a strict requirement - adding keyboard shortcuts should be a case-by-case decision.

## Build Integration

Files in this directory are processed by `tools/cdata.js` into generated headers
(`wled00/html_*.h`, `wled00/js_*.h`).
Run `npm run build` after any change. **Never edit generated headers directly.**
