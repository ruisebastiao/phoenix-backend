import qbs 1.0
import qbs.File

QtApplication {
    id: app;
    name: "frontend_tests"
    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "qml", "sql"] }

    cpp.includePaths: {
        var dirs = ["include"
                    , "../cpp/include"
                    , "../cpp/include/databases"
                    , "../cpp/include/databases/librarydb"
                    , "../cpp/include/databases/openvgdb"
                    , "../cpp/include/models"
                    , "../cpp/include/models/systemlistmodel"
                    , "../cpp/include/models/gamemetadatamodel"
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
        ]
    }

    files: {
        var headers = ["include/*.hpp"
                       , "../cpp/include/*.h"
                       , "../cpp/include/databases/*.h"
                       , "../cpp/include/databases/librarydb/*.h"
                       , "../cpp/include/databases/openvgdb/*.h"
                       , "../cpp/include/models/*.h"
                       , "../cpp/include/models/systemlistmodel/*.h"
                       , "../cpp/include/models/gamemetadatamodel/*.h"]

        var sources = ["src/*.cpp"
                       , "src/models/*.cpp"
                       , "../cpp/src/logging.cpp"
                       , "../cpp/src/databases/*.cpp"
                       , "../cpp/src/databases/librarydb/*.cpp"
                       , "../cpp/src/databases/openvgdb/*.cpp"
                       , "../cpp/src/models/*.cpp"
                       , "../cpp/src/models/systemlistmodel/*.cpp"
                       , "../cpp/src/models/gamemetadatamodel/*.cpp"
                       ];

        return headers.concat( sources );
    }
}
