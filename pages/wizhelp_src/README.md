# Sorgenti wizhelp (staff)

Runtime: `pages/wizhelptbl` (generato in build; **non** editare a mano).

Sorgenti: questa cartella. Ordine: `MANIFEST`.

## Struttura

- `00_index.help` — menù / indici
- `52_commands.help` … `60_commands.help` — comandi per livello minimo
- `90_reference.help` — tabelle (roomflags, razze, danni, …)

## Keyword

Dopo `#`, tutti gli alias sulla **stessa riga**:

```
#
"AUTH" "AUTHORIZE"
```

## Build

CMake target `wizhelptbl` (stesso `helptbl_builder` del player help).
