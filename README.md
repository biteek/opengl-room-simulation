# Real-Time 3D Room Lighting Simulation

Interaktywna symulacja 3D oświetlonego wnętrza zrealizowana w **C++17** oraz **OpenGL 3.3 Core Profile**. 

Aplikacja renderuje w pełni oteksturowaną scenę pokoju z dynamicznym oświetleniem wieloźródłowym, animacjami czasu rzeczywistego (time-based) oraz interaktywną kamerą pierwszoosobową (FPS).

![Prezentacja działania symulacji](showcase.gif)

---

## Cechy projektu i architektura graficzna

* **Model Oświetlenia Phonga:** Własna implementacja modelu oświetlenia we fragment shaderze (komponenty Ambient, Diffuse, Specular z uwzględnieniem tłumienia odległościowego).
* **Obsługa wielu źródeł światła:**
  * **Point Light:** Światło punktowe symulujące żarówkę sufitową z nieliniowym tłumieniem kwadratowym.
  * **Spotlight:** Światło stożkowe/reflektor z gładkim wygaszaniem krawędzi (płynne przejście między kątami odcięcia `innerCutOff` i `outerCutOff`).
  * **Dynamiczne tryby:** Możliwość przełączania w locie między oboma źródłami, wyłącznie stożkowym, wyłącznie punktowym lub całkowitym wygaszeniem.
* **Wczytywanie modeli i tekstur:**
  * Parsowanie geometrii i siatek z plików `.obj` przy użyciu biblioteki `tinyobjloader`.
  * Mapowanie tekstur 2D (`.jpg`) z filtrowaniem dwuliniowym i generowaniem łańcuchów Mipmap za pośrednictwem `stb_image`.
* **Animacje i interakcja:**
  * Płynna, klatkowo niezależna animacja otwierania i zamykania drzwi oparta na delcie czasu (`deltaTime`) i przesuniętym punkcie obrotu (zawias).
  * Ciągły obrót łopatek wentylatora w osi lokalnej.
* **Kamera FPS:** Pełna swoboda ruchu we wszystkich kierunkach z obsługą wejścia z myszy (Pitch/Yaw z ograniczeniami kątowymi).

---

## Sterowanie

| Klawisz / Mysz | Akcja |
| :--- | :--- |
| `W`, `S`, `A`, `D` | Poruszanie się po pokoju (przód, tył, lewo, prawo) |
| `Ruch myszą` | Rozglądanie się (obrót kamery Pitch & Yaw) |
| `E` | Interakcja: płynne otwarcie / zamknięcie drzwi |
| `T` | Przełącznik trybów oświetlenia (Oba $\rightarrow$ Reflektor $\rightarrow$ Punktowe $\rightarrow$ Wyłączone) |
| `ESC` | Wyjście z aplikacji |

---

## Zastosowany stos technologiczny

* **Język:** C++17
* **API Graficzne:** OpenGL 3.3 Core Profile
* **System budowania:** CMake (z modułem `FetchContent` automatycznie pobierającym wymagane biblioteki)
* **Biblioteki:**
  * [GLFW](https://www.glfw.org/) – Zarządzanie oknem kontekstu OpenGL i obsługa wejścia.
  * [GLAD](https://glad.dav1d.de/) – Moduł ładujący wskaźniki funkcji OpenGL 3.3.
  * [GLM](https://github.com/g-truc/glm) – Biblioteka matematyczna (macierze transformacji, wektory, rzutowanie perspektywiczne).
  * [stb_image](https://github.com/nothings/stb) – Dekodowanie plików graficznych `.jpg`.
  * [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) – Parser plików `.obj`.

---

## Budowanie i uruchomienie

### Wymagania wstępne
* Kompilator C++ obsługujący standard C++17 (MSVC, GCC lub Clang)
* CMake w wersji minimum 3.14

### Instrukcja kompilacji

1. Sklonuj repozytorium:
   ```bash
   git clone https://github.com/twoj-login/nazwa-repo.git
   cd nazwa-repo
   ```

2. Skonfiguruj projekt za pomocą CMake:
   ```bash
   cmake -B build
   ```

3. Skompiluj aplikację:
   ```bash
   cmake --build build --config Release
   ```
   *(CMake automatycznie skopiuje katalog `objects/` z zasobami do folderu wynikowego pliku `.exe`)*

4. Uruchom program:
   * **Windows (Visual Studio):** `build/Release/GrafikaKomputerowa.exe` (lub `build/Debug/GrafikaKomputerowa.exe`)
   * **Linux / Ninja:** `build/GrafikaKomputerowa`