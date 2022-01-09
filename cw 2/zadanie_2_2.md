# Ładowanie modeli

w tym zadaniu przećwiczymy ładowanie modeli z plików, wykorzystamy do tego bibliotekę assimp (The Open Asset Import Library ), która zapewnia wspólny interfejs dla różnych typów plików. 

Funkcja `loadModelToContext` pobiera ścieżkę do pliku z modelem i wczytuje go przy użyciu importera assimp.

```c++
const aiScene* scene = import.ReadFile(path, aiProcess_TriangulateaiProcess_Triangulate | aiProcess_CalcTangentSpace);
```

Importer przyjmuje ścieżkę i flagi preprocesingu, które mówią jakie operacje ma wykonać importer przed przekazaniem nam pliku. W naszym przypadku dokonuje triangularyzacji (zamienia wszystkie wielokąty na trójkąty) i oblicza przestrzeń styczną (o której będzie mowa później).

> Wywołaj funkcję dla ścieżki do statku **./models/spaceship.obj** i zmiennej globalnej `Core::RenderContext sphereContext`. Dodaj breakpoint po załadowaniu sceny i obejrzyj jak wygląda struktura załadowanego obiektu

Załadowany obiekt posiada szereg pól jak na przykład tekstury, oświetlenia, materiały, węzły (*Node*) czy modele. Węzły odpowiadają za hierarchię elementów w modelu, co ułatwia jego animację, wykorzystamy to w późniejszych zajęciach, w trakcie tych zajęć będziemy się skupiać na modelach. Nasze obiekty składają się z tylko jednego modelu, wywołaj `context.initFromAiMesh` z nim jako argumentem. 

### Zadanie 

Jeśli tego nie zrobiłeś wywołaj metodę `context.initFromAiMesh`  z argumentem`scene->mMeshes[0]`  po wczytaniu sceny. Metoda nie jest kompletna, uzupełnij ją o ładowanie indeksów, wierzchołków, normalnych i współrzędnych tekstur do bufora. Współrzędne tekstur i indeksy zostały przekonwertowane do odpowiedniego formatu i znajdują się w zmiennych  `std::vector<float> textureCoord` i ` std::vector<unsigned int> indices` odpowiednio. Pozostałe są dostępne jako atrybuty `aiMesh`, mianowicie `mesh->mVertices` zawiera wierzchołki a `mesh->mNormals` normalne

Dodatkowo 

```C++
unsigned int vertexDataBufferSize = sizeof(float) * mesh->mNumVertices * 3;
unsigned int vertexNormalBufferSize = sizeof(float) * mesh->mNumVertices * 3;
unsigned int vertexTexBufferSize = sizeof(float) * mesh->mNumVertices * 2;
```

zawierają rozmiary buforów.

Wykorzystaj w `renderScene` funkcję `Core::DrawContext(Core::RenderContext& context)`

