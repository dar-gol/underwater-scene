# Obsługa PhysX

PhysX jest silnikiem fizyki. Zadaniem silnika fizyki jest przeprowadzenie obliczeń związanych z symulacją zjawisk fizycznych jak dynamika brył sztywnych, płynów lub *soft body dynamics*. Wyręcza on twórcę gry w samodzielnej implementacji fizyki. 

Innymi popularnymi silnikami fizyki są:

* Havok 
* Bullet 
* Advanced Simulation Librar

My wykorzystamy PhysXa do symulacji dynamiki brył sztywnych. Dokumentację można znaleźć w poniższych linkach 

https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/Index.html
https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxapi/files/index.html

## Inicjalizacja PhysX

W tej części przejdziemy po istniejącym kodzie i omówimy jego działanie.

Zaczniemy od klasy `Physics`, która zawiera atrybut `scene`, (która zawiera naszą scenę, czyli obiekty *aktorów*, które mają ze sobą fizycznie reagować i ich własności.) oraz atrybut `physics`, który służy do tworzenia tych obiektów. 

Jej konstruktor wygląda następująco:

```C++
Physics::Physics(float gravity)
{
    foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
    physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true);

    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    // określa siłę i kierunek grawitacji
    sceneDesc.gravity = PxVec3(0.0f, -gravity, 0.0f);
    // definicja dispatchera, który rozdziela zadania do wykonania, tutaj korzystamy z domyślnego dispatchera, któy będzie działał na 2 wątkach
    dispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = dispatcher;
    // definicja filtra, filtr decyduje między jakimi obiektami 
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    scene = physics->createScene(sceneDesc);
}
```



Następnie mamy funkcję `step`, która wykonuje symulację, składa się ona z 2 kroków; najpierw wykonywana jest symulacja, następnie pobierane wyniki. 

Położenie w scenie physXa musimy przenieść do naszej sceny macierze transformacji odpowiadające ich położeniu są pobierane w funkcji

```C++	
void updateTransforms()
{
    // Definiujemy flagę jakie obiekty chcemy pobrać, w tym wypadku statyczne i dynamiczne.
    auto actorFlags = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
    PxU32 nbActors = pxScene.scene->getNbActors(actorFlags);
    if (nbActors)
    {
        //pobieramy obiekty
        std::vector<PxRigidActor*> actors(nbActors);
        pxScene.scene->getActors(actorFlags, (PxActor**)&actors[0], nbActors);
        for (auto actor : actors)
        {
            // Userdata jest atrybutem, któy przechowuje wskaźnik do obiektu zdefiniowanego przez nas, służy do powiądania aktora ze sceny z naszym obiektem. My będziemy tu przechowywać macierz transformacji
            if (!actor->userData) continue;
            glm::mat4 *modelMatrix = (glm::mat4*)actor->userData;

            // pobiera położenie aktora
            PxMat44 transform = actor->getGlobalPose();
            auto &c0 = transform.column0;
            auto &c1 = transform.column1;
            auto &c2 = transform.column2;
            auto &c3 = transform.column3;

            // ustawia wartości macierzy z userData
            *modelMatrix = glm::mat4(
                c0.x, c0.y, c0.z, c0.w,
                c1.x, c1.y, c1.z, c1.w,
                c2.x, c2.y, c2.z, c2.w,
                c3.x, c3.y, c3.z, c3.w);
        }
    }
}
```

## Tworzenie aktora

Aktora tworzy się za pomocą metod klasy `PxPhysics`, jest ona dostępna u nas po `pxScene.physics` (pamiętaj, że jest to wskaźnik i trzeba używać strzałki zamiast kropki do wywołania metod). 

Aby stworzyć aktora należy użyć metody `createRigidStatic` lub `createRigidDynamic`, które tworzą odpowiednio statycznego i dynamicznego aktora (o aktorze statycznym można pomyśleć jak o obiekcie z nieskończoną masą, na którego nie działają żadne siły). Funkcja przyjmuje argument `transform`, który jest początkową pozycją. 

Następnie musimy określić kształt obiektu za pomocą za pomocą metody `createShape` ona z kolei przyjmuje 2 argumenty: geometrię i materiał. Geometria odpowiada za kształt obiektu (przykładowo `PxPlaneGeometry()` daje nam płaszczyznę a `PxBoxGeometry(hx, hy, hz)` daje prostopadłościan o wymiarach 2hx na 2hy na 2hz). Kształt dodaje się do aktora za pomocą metody `attachShape`. Dodany kształt należy usunąć używając jego metody `release`

Dodatkowo do aktora można dodać dodatkowe dane poprzez atrybut `userData`, który jest wskaźnikiem typu `void*`. Wykorzystywany jest by powiązać scenę silnika fizycznego z reprezentacją graficzną, która będzie na ekranie

### Zadanie

Wykonaj zadania opisane w komentarzach pliku **main_8_1**  i **main_8_2** 



## Rejestrowanie zdarzeń

W tej części skupimy się na rejestrowaniu i obsłudze zderzeń pomiędzy obiektami. Jest to przydatna funkcjonalność przy tworzeniu gier lub ogólnie aplikacji wykorzystującej fizykę. Z ich pomocą można zaimplementować szereg mechanik, jak zadanie obrażeń w wyniku trafienia pociskiem/bronią białą, wywołanie animacji czy przytwierdzenie haka do ściany. 

Implementacja wymaga najpierw zdefiniowania swojego *filter shadera*, w którym należy zdefiniować jak mają być obsłużone zdarzenia w scenie physx. W poniższej definicji definiujemy, że punkty zderzenia będą obsługiwane przez `OnContact`.

```C++
static PxFilterFlags simulationFilterShader(PxFilterObjectAttributes attributes0,
    PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
    pairFlags =
        PxPairFlag::eCONTACT_DEFAULT | // default contact processing
        PxPairFlag::eNOTIFY_CONTACT_POINTS | // contact points will be available in onContact callback
        PxPairFlag::eNOTIFY_TOUCH_FOUND; // onContact callback will be called for this pair
        
    return physx::PxFilterFlag::eDEFAULT;
}

```

Samą obsługę zdarzeń definiujemy za pomocą  wywołań zwrotnych - *callbacków*. 

> Wywołanie zwrotne można rozumieć jako odwrotność wywołania funkcji. Zwykle programista wykorzystuje biblioteki poprzez wywoływanie zawartych w niej funkcji. W tym przypadku jest odwrotnie: programista pisze funkcję i przekazuje ją bibliotece, która odpowiada za jej użycie w odpowiednim momencie. 

*Callback* ustala się poprzez ustawienie odpowiedeniego atrybutu:

 `sceneDesc.simulationEventCallback = simulationEventCallback;`

Klasa obiektu `simulationEventCallback` musi dziedziczyć po klasie `PxSimulationEventCallback`, która zawiera szereg metod będących różnymi `callbackami` odpowiadającymi za różne zdarzenia. Nas będzie interesować metoda `onContact`, która jest wywoływana dla każdego zetknięcia się dwóch obiektów w scenie.

### Zadania 

Wykonaj zadania opisane w **main8_3.cpp**. 

Jak zrobisz raportowanie na konsoli zderzeń pomiędzy kulą a kostkami zmodyfikuj kod tak, żeby kostki, które zetkną się z kulą znikały
