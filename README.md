# SISOP-4-2026-IT-023
# LAPORAN

## SOAL 1

### Save Asisten Kenz

Program `kenz_rescue.c` dibuat menggunakan FUSE (Filesystem in Userspace) untuk membuat filesystem virtual yang melakukan passthrough file dari directory sumber `amba_files` ke mount directory `mnt`. Filesystem juga menambahkan virtual file `tujuan.txt` yang dibuat secara on-the-fly tanpa disimpan secara fisik di source directory.

`Poin 1 - Membaca file asli menggunakan passthrough`

Filesystem dibuat agar file `1.txt` hingga `7.txt` yang berada pada directory `amba_files` dapat diakses langsung melalui mount directory `mnt` dengan isi yang identik. Proses passthrough dilakukan menggunakan callback `getattr`, `readdir`, `open`, dan `read`.

Pada callback `read`, file asli dibuka menggunakan `open()` kemudian isi file dibaca menggunakan `pread()` sehingga seluruh isi file pada mount directory sama persis dengan file source.

```c
int fd = open(fullpath, O_RDONLY);

if (fd == -1)
    return -errno;

int res = pread(fd, buf, size, offset);

if (res == -1)
    res = -errno;

close(fd);

return res;
```

`Poin 2 - Menampilkan file pada mount directory`

Callback `readdir` digunakan untuk membaca seluruh isi directory source kemudian menampilkannya pada mountpoint `mnt`. Program menggunakan `opendir()` dan `readdir()` untuk membaca file satu per satu kemudian memasukkannya ke filesystem virtual menggunakan `filler()`.

```c
dp = opendir(fullpath);

while ((de = readdir(dp)) != NULL)
{
    filler(buf, de->d_name, NULL, 0, 0);
}
```

Selain file asli, callback `readdir` juga menambahkan virtual file `tujuan.txt`.

```c
filler(buf, "tujuan.txt", NULL, 0, 0);
```

`Poin 3 - Virtual file tujuan.txt`

Filesystem menambahkan file virtual bernama `tujuan.txt` yang tidak benar-benar ada pada source directory `amba_files`. File ini hanya muncul pada mount directory `mnt`.

Pada callback `getattr`, file virtual dikenali menggunakan pengecekan path.

```c
if (strcmp(path, "/tujuan.txt") == 0)
{
    stbuf->st_mode = S_IFREG | 0444;
    stbuf->st_nlink = 1;
    stbuf->st_size = 66;
    return 0;
}
```

`Poin 4 - Generate isi file secara on-the-fly`

Isi dari `tujuan.txt` tidak disimpan di disk, melainkan dibuat saat file dibaca menggunakan callback `read`. Program membaca seluruh file `1.txt` hingga `7.txt`, mencari fragment yang diawali dengan keyword `KOORD:`, kemudian menggabungkannya menjadi satu string tujuan akhir.

```c
for (int i = 1; i <= 7; i++)
{
    snprintf(filepath,
             sizeof(filepath),
             "%s/%d.txt",
             source_dir,
             i);

    FILE *fp = fopen(filepath, "r");

    while (fgets(line, sizeof(line), fp))
    {
        char *found = strstr(line, "KOORD:");

        if (found != NULL)
        {
            ...
        }
    }

    fclose(fp);
}
```

`Poin 5 - Menghapus newline dan spasi fragment koordinat`

Setelah menemukan fragment `KOORD:`, program menghapus spasi depan dan karakter newline agar seluruh fragment koordinat dapat tergabung dengan rapi menjadi satu kalimat utuh.

```c
char *start = found + 6;

while (*start == ' ')
{
    start++;
}

strcpy(temp, start);

temp[strcspn(temp, "\n")] = 0;

strcat(result, temp);
```

`Poin 6 - Menggunakan snprintf() untuk keamanan buffer`

Seluruh proses pembuatan path menggunakan `snprintf()` agar ukuran buffer tetap aman dan menghindari overflow.

```c
snprintf(fullpath,
         sizeof(fullpath),
         "%s%s",
         source_dir,
         path);
```

`Poin 7 - Menjalankan filesystem FUSE`

Filesystem dijalankan menggunakan mode foreground, debug, dan single-thread agar lebih stabil pada environment WSL.

```bash
sudo ./kenz_rescue -f -d -s mnt
```

Keterangan:

* `-f` : foreground mode
* `-d` : debug mode
* `-s` : single-thread mode

---

### REVISI

Sebelumnya isi `tujuan.txt` masih menghasilkan format yang salah karena fragment koordinat masih mengandung leading space dan newline sehingga output menjadi terpisah-pisah dan jumlah karakter tidak sesuai dengan indikator soal (`wc -c = 66`).

`Before`

```c
strcpy(temp, found + 6);

temp[strcspn(temp, "\n")] = 0;

strcat(result, temp);
```

Masalah:

* masih mengambil spasi setelah `KOORD:`
* output koordinat menjadi tidak rapi
* jumlah karakter tidak sesuai

`After`

```c
char *start = found + 6;

while (*start == ' ')
{
    start++;
}

strcpy(temp, start);

temp[strcspn(temp, "\n")] = 0;

strcat(result, temp);
```

Perbaikan:

* menghapus leading space
* menghapus newline
* hasil koordinat menjadi satu baris utuh
* output sesuai indikator soal (`66 byte`)

---

## OUTPUT SOAL 1

`Mount directory berhasil muncul`

```bash id="rm0lpc"
sudo ls mnt
```

Output:

1.txt
2.txt
3.txt
4.txt
5.txt
6.txt
7.txt
tujuan.txt

![output mount](assets/mount.png)

---

`Test passthrough file`

```bash id="zc3dnr"
sudo cat mnt/1.txt
```

Output file identik dengan source `amba_files/1.txt`.

![output passthrough](assets/passthrough.png)

---

`Test virtual file`

```bash id="zcb1bh"
sudo cat mnt/tujuan.txt
```

Output:

Tujuan Mas Amba: -7.957382728443728,112.4698688227961,23:59 WIB

![output tujuan](assets/tujuan.png)

---

`Test ukuran file virtual`

```bash id="qkdnr5"
sudo wc -c mnt/tujuan.txt
```

Output:

66 mnt/tujuan.txt

![output wc](assets/wc.png)

---

`Struktur folder saat runtime`

```bash id="icoc0n"
tree
```

Output:

.
├── amba_files
│   ├── 1.txt
│   ├── 2.txt
│   ├── 3.txt
│   ├── 4.txt
│   ├── 5.txt
│   ├── 6.txt
│   └── 7.txt
├── kenz_rescue
├── kenz_rescue.c
└── mnt

![output tree](assets/tree.png)

## KENDALA

Kendala utama terdapat pada environment WSL dan permission FUSE. Awalnya filesystem tidak dapat membaca source directory karena mountpoint dijalankan menggunakan root sehingga permission antara user dan root bercampur. Selain itu callback `readdir` sempat gagal membaca source directory akibat path relative yang tidak stabil pada WSL.

Masalah lain terjadi pada virtual file `tujuan.txt` dimana hasil penggabungan fragment koordinat masih mengandung spasi dan newline sehingga output tidak sesuai dengan indikator soal. Permasalahan diselesaikan dengan melakukan trimming leading space dan menghapus karakter newline sebelum fragment digabungkan.
