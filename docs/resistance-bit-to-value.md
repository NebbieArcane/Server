# Resistenze: conversione bitvector → scala -100..+100

Riferimento per `legacy_loader`, script di migrazione e adapter futuro.  
Sorgente bit: `char_data.immune`, `char_data.M_immune`, `char_data.susc` (`structs.hpp`).  
Destinazione: `character_resistance` (`damage_type`, `value`).

## Scala `value`

| `value` | Significato (obiettivo gameplay) |
|---------|-----------------------------------|
| `-100` | Suscettibile — danni raddoppiati |
| `0` | Normale |
| `+1` … `+99` | Resistente (gradiente; da definire in combat) |
| `+100` | Immune |

Formula danni effettivi: da implementare in codice (es. moltiplicatore da `value`).

## Regola migrazione da bitvector (una tantum)

Per ogni bit `IMM_*` in `autoenums.hpp`, per un dato PG:

| Condizione (priorità dall’alto) | `value` |
|----------------------------------|---------|
| Bit presente in `immune` **oppure** `M_immune` | `+100` |
| Bit presente in `susc` (e non immune) | `-100` |
| Nessun bit | `0` |

Se lo stesso bit è sia in `susc` che in `immune`, vince **immune** → `+100`.

**Nota:** il vecchio sistema non ha uno stato “resistente +50” sui bit; dopo migrazione tutti partono da -100 / 0 / +100. I valori intermedi si impostano solo con il nuovo sistema (equip, spell, editor).

## `damage_type` (valore = bit `IMM_*`)

| `damage_type` | Nome | Categoria | Fisico / Magico |
|---------------|------|-----------|-----------------|
| 1 | IMM_FIRE | elementale | magico |
| 2 | IMM_COLD | elementale | magico |
| 4 | IMM_ELEC | elementale | magico |
| 8 | IMM_ENERGY | elementale | magico |
| 16 | IMM_BLUNT | arma | **fisico** |
| 32 | IMM_PIERCE | arma | **fisico** |
| 64 | IMM_SLASH | arma | **fisico** |
| 128 | IMM_ACID | elementale | magico |
| 256 | IMM_POISON | stato | magico |
| 512 | IMM_DRAIN | stato | magico |
| 1024 | IMM_SLEEP | mentale | magico |
| 2048 | IMM_CHARM | mentale | magico |
| 4096 | IMM_HOLD | mentale | magico |
| 8192 | IMM_NONMAG | meta | meta |
| 16384 | IMM_PLUS1 | arma +1 | meta |
| 32768 | IMM_PLUS2 | arma +2 | meta |
| 65536 | IMM_PLUS3 | arma +3 | meta |
| 131072 | IMM_PLUS4 | arma +4 | meta |

Fonte: `src/autoenums.hpp` (`E_IMMUNITY_TYPE`).

## Persistenza oggi vs domani

| Oggi | Domani |
|------|--------|
| PG: bit solo in RAM; save indiretto via `character_affects` (`APPLY_IMMUNE` 26, `APPLY_SUSC` 27, `APPLY_M_IMMUNE` 28) | `character_resistance` = profilo base al save |
| Mob: `immune`/`M_immune`/`susc` in file `.mob` | Schema mondo (futuro) o stessa tabella per prototipi |
| `.dat` senza campi resistenza | Import: 0 su tutti i tipi, poi script da affect o da mob template |

## Affect temporanei (transizione)

Fino al refactor combat:

- `character_affects` con `location` 26/27/28 e `modifier` = bit `IMM_*` restano validi per buff temporanei.
- A regime: buff modificano `value` in RAM e/o riga in `character_resistance` con flag “temporaneo” (futuro).

## Pseudocodice migrazione (un PG)

```
for each damage_type D in IMM_TABLE:
  if (immune & D) or (M_immune & D):
    value = 100
  else if (susc & D):
    value = -100
  else:
    value = 0
  INSERT character_resistance (toon_id, damage_type, value)
  ON DUPLICATE KEY UPDATE value = VALUES(value)
```

## SQL di esempio (dopo load in tabella staging o da app)

Non eseguire alla cieca: richiede colonne `immune`/`susc`/`m_immune` in staging. Usare da codice C++ in `legacy_loader` è preferibile.
