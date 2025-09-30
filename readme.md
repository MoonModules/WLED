<p align="center">
  <img src="/images/wled_logo_akemi.png">
  <a href="https://github.com/MoonModules/WLED/releases"><img src="https://img.shields.io/github/release/MoonModules/WLED.svg?style=flat-square"></a>
  <a href="https://raw.githubusercontent.com/MoonModules/WLED/mdev/LICENSE"><img src="https://img.shields.io/github/license/MoonModules/wled?color=blue&style=flat-square"></a>
  <a href="https://wled.discourse.group"><img src="https://img.shields.io/discourse/topics?colorB=blue&label=forum&server=https%3A%2F%2Fwled.discourse.group%2F&style=flat-square"></a>
  <a href="https://discord.gg/4CQRmfR"><img src="https://img.shields.io/discord/700041398778331156.svg?colorB=blue&label=discord&style=flat-square"></a>
  <a href="https://mm.kno.wled.ge"><img src="https://img.shields.io/badge/quick_start-wiki-blue.svg?style=flat-square"></a>
  <a href="https://github.com/Aircoookie/WLED-App"><img src="https://img.shields.io/badge/app-wled-blue.svg?style=flat-square"></a>
  <a href="https://gitpod.io/#https://github.com/MoonModules/WLED"><img src="https://img.shields.io/badge/Gitpod-ready--to--code-blue?style=flat-square&logo=gitpod"></a>

  </p>

# Welcome to WLED MoonModules! ✨

<img width="400" alt="image" src="https://user-images.githubusercontent.com/91013628/230378884-5a0f15ee-1aa2-4998-9df7-ade9f32a3d0f.png">

## ⚡ Custom Modifications in this Fork

### Extended Segment Name Length (128 Characters)
This fork extends the segment name length from the default **32 characters** to **128 characters**, allowing for much longer, more descriptive segment names.

**What's Changed:**
- Segment names can now be up to 128 characters long (previously 32)
- Full support in text scroll LED effects
- All backend buffers and validations updated
- Frontend input fields support the extended length

**Modified Files:**
- `wled00/const.h` - Added `WLED_MAX_SEGNAME_LEN` constant (128)
- `wled00/json.cpp` - Updated validation logic
- `wled00/FX.cpp` - Fixed text scroll effect buffer for longer names
- `wled00/FX_fcn.cpp` - Updated jMap filename buffer
- `wled00/image_loader.cpp` - Updated filename buffers
- `wled00/data/index.js` - Updated input field `maxlength` attribute

**Use Case:**
Perfect for installations with many segments where descriptive names are essential, or when using the text scroll effect to display longer messages on LED matrices.

**⚠️ Disclaimer:**
This modification was primarily developed and tested for the **text scroll LED effect** on matrices. While all related code paths have been updated to support 128-character names, not all features and effects have been extensively tested with very long segment names. Use at your own risk.

---

MoonModules/WLED is a fork of [Aircoookie/WLED](https://github.com/Aircoookie/WLED) which contains latest merge of v0.14 of WLED with [additional features](https://mm.kno.wled.ge/moonmodules/what-is-moonmodules/).

This fork is created by members of the [Atuline/WLED](https://github.com/atuline/WLED) team to make development against v0.14 possible while still preserving [Atuline/WLED v0.13.x](https://github.com/atuline/WLED/tree/dev) as a stable and supported version. The Atuline/WLED fork is also called WLED SR (Sound Reactive).

More info here: <a href="https://mm.kno.wled.ge/moonmodules/what-is-moonmodules/">what-is-moonmodules</a>

<a href="https://www.paypal.com/donate?business=moonmodules@icloud.com"><img src="https://img.shields.io/badge/send%20me%20a%20small%20gift-paypal-blue.svg" alt="HTML tutorial" style="max-width: 100%;"></a>
Donations will be used to buy WLED related hardware, software or drinks shared with the contributors of this repo.

## License 
WLED-MM is licensed under the [EUPL-1.2](https://joinup.ec.europa.eu/collection/eupl) or later. 
The official license text is [available in 23 languages](https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12).

## Contributing
We welcome contributions to this project! See [contributing](https://github.com/MoonModules/WLED/blob/mdev/CONTRIBUTING.md) for more information.
> We would like to have this repository in a polite and friendly atmosphere, so please be kind and respectful to others. For more details, look at [Code of Conduct](https://github.com/MoonModules/WLED/blob/mdev/CODE_OF_CONDUCT.md).

## *Disclaimer:*   

Using this software is the users responsibility as it is not bug free. Therefore contributors of this repo are not reliable for anything including but not limited to spontaneous combustion of the entire led strip, the house and the inevitable heat death of the universe
