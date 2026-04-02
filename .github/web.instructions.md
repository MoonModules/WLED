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

## Build Integration

Files in this directory are processed by `tools/cdata.js` into `wled00/html_*.h` headers.
Run `npm run build` after any change. **Never edit the generated `html_*.h` files directly.**
