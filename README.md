# SISTEMI EMBEDDED REAL TIME - ESERCIZI ESAME SCRITTO C
La cartella `guide/` contine alcune linee guida di pseudo-codice per semafori (prologo / epilogo di soluzione1 / soluzione2) e per mutex.

La cartella `bin/` contiene i binari compilati.

La cartella `svolti/` contiene le soluzioni proposte degli esercizi `testi.pdf`.

La cartella `soluzioni/` contiene le soluzioni proposte dal prof.

Il `Dockerfile` serve a chi ha Mac OS X, per lavorare all'interno di un container ubuntu con il `gcc` di Linux. Questo perchè su Mac alcune funzioni dei semafori come la `sem_init` sono deprecate e non funzionanti.  

----------------

### Compile file.c
 ```sh
 $ make compile FILE=[file.c] DIR=[./svolti/]
 ```
 ----------------

### Build Image
```sh
$ make build
```
----------------

### Run Container
```sh
$ make run
```
----------------

### Build & Run
```sh
$ make all
```
----------------
