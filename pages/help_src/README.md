# Sorgenti help giocatore

Convenzione (canonica + indici):

1. **Skill/spell**: testo una sola volta (`30_skills.help`, `40_spells.help`).
2. **Classi** (`classes/`): `00_index.help` + **un file per classe** (indice/link, niente body duplicati).
3. **Razze** (`races/`): `00_index.help` + **un file per razza**.
4. **Sistemi** (`20_systems.help`): multiclasse, newbie, practice, group, ecc.
5. **Legacy** (`99_legacy.help`): ancora da riclassificare.

## Build

`mudroot/pages/helptbl` si rigenera in automatico con CMake (`helptbl_builder`), **solo se** il contenuto di `pages/help_src` cambia (stamp `mudroot/pages/helptbl.stamp`).

Sorgenti: `pages/help_src/`. Output runtime: `mudroot/pages/helptbl`.

Ordine di concatenazione: `MANIFEST`.

Editare **solo** i file sotto `pages/help_src/`; non modificare `helptbl` a mano.
