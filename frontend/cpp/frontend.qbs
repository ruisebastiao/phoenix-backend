import qbs 1.0
import qbs.File

QtApplication {
    id: app;
    name: "frontend"
    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "qml", "sql"] }

    cpp.includePaths: {
        var dirs = [ "include"
                    , "include/databases"
                    , "include/databases/librarydb"
                    , "include/databases/openvgdb"
                    , "include/models"
                    , "include/models/gamemetadatamodel"
                    , "include/models/systemlistmodel"
                    ];
        return dirs;
    }

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

    cpp.cxxLanguageVersion: "c++14";
    cpp.dynamicLibraries: [ "mingw32" ]

    files: {
        var headers = ["include/*.hpp"
                       , "include/*.h"
                       , "include/models/*.h"
                       , "include/models/gamemetadatamodel/*.h"
                       , "include/models/systemlistmodel/*.h"
                       , "include/databases/*.h"
                       , "include/databases/librarydb/*.h"
                       , "include/databases/openvgdb/*.h"];

        var qmlFiles = ["qml/*.qrc"
                        , "qml/icons/*.qrc"];

        var sources = ["src/*.cpp"
                       , "src/models/*.cpp"
                       , "src/models/gamemetadatamodel/*.cpp"
                       , "src/models/systemlistmodel/*.cpp"
                       , "src/databases/*.cpp"
                       , "src/databases/librarydb/*.cpp"
                       , "src/databases/openvgdb/*.cpp"];

        return headers.concat( sources ).concat(qmlFiles);
    }
}
