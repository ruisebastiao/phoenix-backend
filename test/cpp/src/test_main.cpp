#define CATCH_CONFIG_RUNNER  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

int main(int argc, char** argv)
{
  return Catch::Session().run(argc, argv);
}

//#include "emulatorintegrationtest.hpp"
//#include "emulationunittests.hpp"

//#include <QDebug>
//#include <QCoreApplication>
//#include <QtTest>


//int main(int argc, char *argv[])
//{
//    QCoreApplication app( argc, argv );

//    app.setAttribute(Qt::AA_Use96Dpi, true);

//    QTEST_SET_MAIN_SOURCE_PATH

//    QList<QObject *> integrationTests = {
////        new EmulatorIntegrationTest(),
////        new EmulatorUnitTests(),
//    };

//    int status = 0;
//    for ( QObject *test : integrationTests ) {
//        status |= QTest::qExec(test, argc, argv);
//        delete test;
//    }

//    if ( status == 0 ) {
//        qDebug() << "\nAll tests have successfully passed!";
//    }

//    return 0;

//}
