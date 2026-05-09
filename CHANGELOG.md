# Changelog

## [1.3.4] - 2026-05-08

### Added
- mo5_font6: support new caracters (+ @ ')
- mo5_font8: support new caracters (+ @ ')

### Fixed
- mo5_font6: mo5_font6_puts() now automatically clears previous text before drawing 
- mo5_font8: mo5_font8_puts() now automatically clears previous text before drawing 

## [1.3.3] - 2026-04-28

### Added
- add mo5_music_swi lib
- add mo5_joystick lib

### Changed
- mo5_hadware_reference.md: update the documentation

### Fixed
- mo5_audio: Fix registry addresses

## [1.3.2] - 2026-04-25

### Added
- add mo5_audio lib
- add mo5_hadware_reference.md documentation

## [1.3.1] - 2026-04-12

### Changed
- README.md: update MCP server reference, following the MCP server deployment

## [1.3.0] - 2026-04-06

### Changed
- Add makefd.py python script, in ordre to create a floppy image from a bin file (replace Olivier P project)

## [1.2.5] - 2026-04-06

### Added
- guide-graphical-development-mo5.md
- mo5_optimization_guide.md

## [1.2.4] - 2026-03-07

### Added
- mo5_font6 documentation (markdown file)
- mo5_font8 documentation (markdown file)

## [1.2.3] - 2026-03-06

> ⚠️ BREAKING CHANGE 

### Changed
- mo5_font6: ty in pixel, to be compliant with the rest of the api
- mo5_font8: ty in pixel, to be compliant with the rest of the api

## [1.2.2] - 2026-03-04

### Added
- Add mo5_font6 lib
- Add mo5_font8 lib

## [1.2.1] - 2026-03-03

### Changed
- libs optimisations

## [1.2.0] - 2026-03-01

### Added
- Add mo5_actor_rd lib (sprite - dirty rectancle)
- Add mo5_sprite_bg lib (sprit - transparent background)
- Add mo5_sprite_form lib (sprite - no color change, form only)
- Add mo5_sprite_type lib (common lib for sprite management)

### Changed
- libs optimisations