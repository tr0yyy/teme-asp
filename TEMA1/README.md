# Arhitectura Sistemelor Paralele
## Tema 1

Cerinta este in "how_to.pdf"\

Pentru a compila:
```bash
mpicc main.c pgm_IO.h pgm_IO.c -o main
```
Pentru a executa:
```bash
mpiexec -n M main T
```
Unde M poate sa fie 1, 2, 4, 8, 10, 12, 16 ; iar T poate sa fie 20, 200, 400, 600, 800, 1000, 1200 sau 1400\

Pentru a verifica imaginea se foloseste functia ```display```, disponibila in pachetul ```imagemagick```

Utilizare:
```bash
display output_image_640x480_T.pmg
```
unde T este cel de mai sus utilizat la executie.
