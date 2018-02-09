import qbs 1.0
import qbs.File

QtApplication {
    id: app;
    name: "frontend_tests"
    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "qml", "quick", "sql", "concurrent", "test"] }

    cpp.includePaths: {
        var dirs = ["./include"
                    , "../cpp/include"
                    , "../cpp/include/databases"
                    , "../cpp/include/databases/librarydb"
                    , "../cpp/include/databases/openvgdb"
                    , "../cpp/include/databases/systemdb"
                    , "../cpp/include/models"
                    , "../cpp/include/models/systemlistmodel"
                    , "../cpp/include/models/gamemetadatamodel"
                    , "../cpp/include/models/inputdevicesmodel"
                    , "../cpp/include/gameimporter"
                ];
        dirs.push( "include")
        return dirs;
    }

    cpp.cxxLanguageVersion: "c++14";
    cpp.dynamicLibraries: [ "mingw32" ]
    destinationDirectory: "phoenix/" + app.name;

    Group {
     name: "Databases";
     qbs.install: true;
     qbs.installDir: "databases";
     qbs.installPrefix: app.name;
     files: [
            "externals/*.sqlite",
            "include/doctest.cpp",
            "include/doctest.h",
            "include/doctestrunner.h",
            "src/GameImporterIntegrationTests.cpp",
            "src/doctestrunner.cpp",
        ]
    }

    Group {
     name: "External Files";
     qbs.install: true;
     qbs.installDir: "testFiles";
     qbs.installPrefix: app.name;
     files: [
            "externals/*.nes",
        ]
    }

    files: {
        var headers = ["include/*.hpp"
                       , "include/*.h"
                       , "../cpp/include/*.h"
                       , "../cpp/include/databases/*.h"
                       , "../cpp/include/databases/librarydb/*.h"
                       , "../cpp/include/databases/openvgdb/*.h"
                       , "../cpp/include/databases/systemdb/*.h"
                       , "../cpp/include/models/*.h"
                       , "../cpp/include/models/systemlistmodel/*.h"
                       , "../cpp/include/models/gamemetadatamodel/*.h"
                       , "../cpp/include/models/inputdevicesmodel/*.h"
                       , "../cpp/include/gameimporter/*.h"
                ]

        var sources = ["src/*.cpp"
                       , "src/models/*.cpp"
                       , "../cpp/src/*.cpp"
                       , "../cpp/src/logging.cpp"
                       , "../cpp/src/databases/*.cpp"
                       , "../cpp/src/databases/librarydb/*.cpp"
                       , "../cpp/src/databases/openvgdb/*.cpp"
                       , "../cpp/src/databases/systemdb/*.cpp"
                       , "../cpp/src/models/*.cpp"
                       , "../cpp/src/models/systemlistmodel/*.cpp"
                       , "../cpp/src/models/gamemetadatamodel/*.cpp"
                       , "../cpp/src/models/inputdevicesmodel/*.cpp"
                       , "../cpp/src/gameimporter/*.cpp"
                       ];

        return headers.concat( sources );
    }
}
