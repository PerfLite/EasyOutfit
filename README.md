<div align="center">

<img src="assets/banner.svg" alt="EasyOutfit" width="100%">

<p>
  <a href="#english">🇬🇧 English</a> ·
  <a href="#русский">🇷🇺 Русский</a>
</p>

<p>
  <img src="https://img.shields.io/github/v/release/PerfLite/EasyOutfit?style=flat-square" alt="release">
  <img src="https://img.shields.io/github/license/PerfLite/EasyOutfit?style=flat-square" alt="license">
  <img src="https://img.shields.io/github/languages/top/PerfLite/EasyOutfit?style=flat-square" alt="language">
  <img src="https://img.shields.io/github/stars/PerfLite/EasyOutfit?style=flat-square" alt="stars">
  <img src="https://img.shields.io/github/downloads/PerfLite/EasyOutfit/total?style=flat-square" alt="downloads">
</p>

<p>
  <a href="https://github.com/PerfLite/EasyOutfit/releases/latest"><b>⬇️ Download / Скачать</b></a> ·
  <a href="https://github.com/PerfLite/EasyOutfit/releases">📦 All releases / Все релизы</a>
</p>

</div>

## ⬇️ Download / Скачать

| File / Файл | Description / Описание | Link / Ссылка |
|---|---|---|
| `EasyOutfit-v1.0.0.zip` | Main mod release (SE & AE) · Основной архив мода (SE и AE) | [Download](https://github.com/PerfLite/EasyOutfit/releases/latest/download/EasyOutfit-v1.0.0.zip) |

> Older versions and changelogs: [Releases](https://github.com/PerfLite/EasyOutfit/releases) ·
> Старые версии и список изменений: [Релизы](https://github.com/PerfLite/EasyOutfit/releases)

---

## English

Modern, lightweight transmogrification and visual cosmetic outfit plugin for Skyrim Special Edition and Anniversary Edition (SKSE64). Wear any gear for optimal stats while maintaining your preferred cosmetic appearance.

### Features

- **Seamless transmogrification**: replace the visual look of equipped armor and clothing without losing stats or enchantments.
- **In-game ImGui overlay**: intuitive interface (toggle with `F1` by default, configurable).
- **Multi-slot support & presets**: customize head, body, hands, feet, shield, amulets, rings, and switch between presets seamlessly.
- **Form transformation safety**: automatically suppresses cosmetic overrides during Werewolf (Beast Form) and Vampire Lord transformations.
- **JSON localization**: simple multi-language support (English, Russian).

### Install

Install using Mod Organizer 2, Vortex, or unpack the archive directly into your Skyrim `Data` directory:

```
https://github.com/PerfLite/EasyOutfit
```

**Requirements:**
- Skyrim Special Edition / Anniversary Edition (`1.5.97`, `1.6.640`, `1.6.1170+`)
- [SKSE64](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)

### Build

```bash
xmake f -m release -y
xmake -y
```

### License

MIT — see [LICENSE](LICENSE).

<p align="right"><a href="#english">↑ Back to top</a> · <a href="#русский">Русский ↓</a></p>

---

## Русский

Современный и легковесный SKSE64-плагин трансмогрификации (косметических нарядов) для Skyrim Special Edition и Anniversary Edition. Носите любые доспехи ради защиты и характеристик, сохраняя любимый внешний облик.

### Возможности

- **Бесшовный трансмог**: замена визуального вида надетой брони и одежды без потери статов и зачарований.
- **Внутриигровое меню ImGui**: удобное всплывающее окно управления (открытие по умолчанию по клавише `F1`, настраивается).
- **Поддержка пресетов и всех слотов**: настройка шлема, кирасы, перчаток, сапог, щита, колец и амулетов с мгновенным переключением.
- **Совместимость с формами зверя и вампира**: автоматическое отключение визуального слоя при превращении в оборотня или Лорда-вампира.
- **JSON-локализация**: полная поддержка перевода интерфейса (Русский, English).

### Установка

Установите через Mod Organizer 2, Vortex или распакуйте архив мода напрямую в папку `Data` игры:

```
https://github.com/PerfLite/EasyOutfit
```

**Требования:**
- Skyrim Special Edition / Anniversary Edition (`1.5.97`, `1.6.640`, `1.6.1170+`)
- [SKSE64](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)

### Сборка

```bash
xmake f -m release -y
xmake -y
```

### Лицензия

MIT — см. [LICENSE](LICENSE).

<p align="right"><a href="#русский">↑ Наверх</a> · <a href="#english">English ↑</a></p>
