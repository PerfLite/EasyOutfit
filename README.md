# EasyOutfit

Современный SKSE плагин трансмогрификации (косметической смены экипировки) для **Skyrim Special Edition (1.5.97)** и **Anniversary Edition (1.6+)**.
Создан с использованием **CommonLibSSE-ng**, **Xbyak** и **SKSE Menu Framework** (ImGui).

---

## Возможности

1. **Косметические наряды (Transmog)**:
   - Сохраняйте любые комбинации брони, шлемов, перчаток, сапог, плащей и щитов как косметические наряды.
   - Носите броню с лучшими характеристиками, выглядя при этом в любом любимом стиле.
   - Поддержка создания нескольких пресетов нарядов, их быстрого переключения, дублирования и удаления.

2. **Современный оверлейный интерфейс**:
   - Интеграция в **SKSE Menu Framework** (быстрый доступ через меню модов или по горячей клавише).
   - Прямой поиск по всей базе брони игры и установленных модов прямо из интерфейса.
   - Добавление элементов наряда прямо из инвентаря персонажа в один клик.

3. **Техническая совместимость и стабильность**:
   - Автоматическое скрытие ванильных мешей при активном наряде через хуки конвейера рендеринга (`ActorWeightModel`).
   - Сохранение корректных масок слотов тела (`BipedObjectSlot`) для предотвращения клиппинга и невидимости частей тела.
   - Корректная обработка формы зверя (оборотень / лорд-вампир) — наряд автоматически скрывается при трансформации.
   - Пресеты сохраняются в `Data/SKSE/Plugins/EasyOutfit_Presets.json` в формате FormID + имя ESP/ESL плагина, что гарантирует сохранность данных при изменениях порядка загрузки.

4. **Локализация**:
   - Полная поддержка русского и английского языков (автоопределение языка игры + переключатель в настройках).

---

## Требования / Requirements

- [SKSE64](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- [SKSE Menu Framework](https://www.nexusmods.com/skyrimspecialedition/mods/109867)

---

## Управление по умолчанию

- Горячая клавиша быстрого включения / отключения наряда: **`O`** (код клавиши 24 в `EasyOutfit.json`)
- Открытие меню: через главное меню **SKSE Menu Framework** (по умолчанию клавиша `F10` или через меню паузы).

---

## Структура файлов

- `SKSE/Plugins/EasyOutfit.dll` — плагин SKSE.
- `SKSE/Plugins/EasyOutfit.json` — конфигурация настроек.
- `SKSE/Plugins/EasyOutfit_Presets.json` — сохранённые наряды игрока (создаётся автоматически в игре).

---

# English Description

**EasyOutfit** is a modern, lightweight, script-free transmogrification (cosmetic outfit overlay) SKSE plugin for **Skyrim Special Edition (1.5.97)** and **Anniversary Edition (1.6+)**.

### Features:
- **Transmogrification**: Wear your best gear for stats while looking however you want.
- **In-Game ImGui UI**: Powered by [SKSE Menu Framework](https://www.nexusmods.com/skyrimspecialedition/mods/109867). Browse, search by name, filter by body slot (head, body, hands, feet, shield, other), or equip items from your inventory.
- **Multiple Outfits**: Save multiple presets, duplicate, rename, or delete them.
- **Transformations Support**: Automatically suppresses outfit overlay while transformed into Werewolf or Vampire Lord form.
- **Zero Script Lag**: Pure C++ engine hooks, zero Papyrus overhead, completely safe to install or uninstall anytime.
- **Bilingual**: Supports English and Russian with auto-detection.

