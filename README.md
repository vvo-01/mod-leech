# mod-leech

An AzerothCore module that heals players when they deal damage.

## Features

- Heals the attacker for a configurable fraction of damage dealt
- Works for player damage and pet damage
- Optional dungeon-only restriction
- Optional required item to activate leech
- Configurable pet leech target: pet, owner, or disabled
- Optional self-damage leech (reflect, fall damage, etc.)
- Config cached on load and refreshed on `.reload config`

## Installation

1. Place the `mod-leech` folder into `modules/`.
2. Copy `conf/leech.conf.dist` to `configs/modules/leech.conf`.
3. Rebuild the server.

## Configuration

File: `configs/modules/leech.conf`

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `Leech.Enable` | bool | `0` | Enable or disable the module |
| `Leech.DungeonsOnly` | bool | `1` | Only leech in dungeons and raids |
| `Leech.Amount` | float | `0.05` | Fraction of damage dealt applied as heal (`0.5` = 50%, `1` = 100%) |
| `Leech.AllowSelfDamage` | bool | `0` | Allow leech from self-inflicted damage |
| `Leech.RequiredItemId` | uint | `0` | Item ID required in bags to activate leech (`0` = no item) |
| `Leech.PetDamage` | string | `pet` | Pet damage leech target: `pet`, `owner`, or `none` |

### PetDamage modes

| Value | Behavior |
|-------|----------|
| `pet` | Pet heals itself from its own damage |
| `owner` | Owner heals from pet damage (threat goes to pet) |
| `none` | Pet damage does not trigger leech |

## How it works

The module hooks `UnitScript::OnDamage` and applies a heal to the attacker (or pet/owner, depending on config) equal to `Leech.Amount × damage`.

Checks are ordered cheap-first:

1. Attacker exists and module enabled
2. Self-damage filter
3. Attacker is player or player's pet
4. Dungeon-only filter
5. Required item filter
6. Heal applied

Config values are cached via `WorldScript::OnAfterConfigLoad`, so no config lookups occur on the damage hot path. Changes apply on `.reload config` without server restart.

## Credits

Based on the original [mod-leech](https://github.com/ZhengPeiRu21/mod-leech) by ZhengPeiRu21.

## License

MIT
