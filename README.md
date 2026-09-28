# SelectFolder

DLL nativa Win32/x86 para Visual FoxPro 9 que muestra el diálogo moderno de Windows para seleccionar una carpeta.

## Función exportada

```c
int __cdecl SelectFolder(
    HWND hOwner,
    char* cFolder,
    int nBufferSize,
    const char* cInitialFolder,
    const char* cTitle);
```

- `hOwner`: HWND del formulario propietario.
- `cFolder`: buffer que recibe la carpeta seleccionada.
- `nBufferSize`: tamaño del buffer.
- `cInitialFolder`: carpeta inicial. Si está vacía o no existe, se usa el directorio actual.
- `cTitle`: título del diálogo. Si está vacío, se usa `Seleccionar carpeta`.
- Retorna `1` si se seleccionó una carpeta y `0` si se canceló o ocurrió un error.

## Visual FoxPro 9

```foxpro
lcDLL = FULLPATH("SelectFolder.dll")

DECLARE INTEGER SelectFolder IN (m.lcDLL) ;
    INTEGER hOwner, ;
    STRING @cFolder, ;
    INTEGER nBufferSize, ;
    STRING cInitialFolder, ;
    STRING cTitle

lcFolder = SPACE(1024)

lnResult = SelectFolder( ;
    Thisform.HWnd, ;
    @lcFolder, ;
    LEN(lcFolder), ;
    "", ;
    "Seleccione una carpeta")

IF m.lnResult = 1
    lcFolder = LEFT( ;
        m.lcFolder, ;
        AT(CHR(0), m.lcFolder + CHR(0)) - 1)
ENDIF
```

## Compilación

El repositorio está preparado para AppVeyor con Visual Studio 2022.

La compilación es:

- Configuration: `Release`
- Platform: `Win32`
- Toolset: `v143`
- Runtime C/C++: estático (`/MT`)
- Salida: `SelectFolder.dll`

No requiere paquetes NuGet ni DLL adicionales.
