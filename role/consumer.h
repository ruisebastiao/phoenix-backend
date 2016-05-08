#pragma once

#include "backendcommon.h"

#include "producer.h"

#include "logging.h"

/*
 * Functionality and structures common to all consumers.
 *
 * To declare these slots in an abstract class you must use the define provided:
 * class AbstractConsumerSubclass : public QObject, public Consumer {
 *     public slots:
 *         CONSUMER_SLOTS_ABSTRACT
 *         void anyOtherSlots();
 * };
 *
 * You can then override them in whatever subclass of the abstract class like normal.
 *
 * To connect to these slots, use the macro provided in "producer.h" or just follow this form:
 * connect( dynamic_cast<QObject *>( ProducerSubclassPtr ), SIGNAL( producerSignal( argType, anotherArgType ) ),
 *          dynamic_cast<QObject *>( ConsumerSubclassPtr ), SLOT( consumerSlot( argType, anotherArgType ) ) );
 *
 * Check out producer.h for more information
 *
 * Thanks to peppe and thiago from #Qt on Freenode for the idea
 */

// In order to properly declare the slots in your abstract subclass of Consumer, simply use this macro under "public slots:"
// Documented below
#define CONSUMER_SLOTS_ABSTRACT \
    virtual void consumerFormat( ProducerFormat consumerFmt ) override = 0; \
    virtual void consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) override = 0;

class Consumer {

    public:
        Consumer();
        ~Consumer();

        // Information about the type of data to expect
        virtual void consumerFormat( ProducerFormat consumerFmt ) = 0;

        // Must obtain a mutex to access the data. Only hold onto the mutex long enough to make a copy
        // Type can be one of the following: "audio", "video", "input", "touchinput"
        // You can check the timestamp against QDateTime::currentMSecsSinceEpoch() to see how far out of sync you might be
        virtual void consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) = 0;

    protected:
        // Data given by the producer
        ProducerFormat consumerFmt;

        // Print statistics every (printEvery) times this function is called
        void printFPSStatistics( int printEvery = 60 );

};