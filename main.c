#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include <sys/time.h>
#include <unistd.h>

volatile int finished = 0;
int subscribed = 0;
int disconnected = 0;

char *topic     = "kaa/#";
char *clientid  = "paho-c-sub";
int qos = 0;
int keepalive = 10;
int MQTTVersion = MQTTVERSION_DEFAULT;

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    printf("%d %s\t", message->payloadlen, topicName);
    printf("%.*s\n", message->payloadlen, (char*)message->payload);
    fflush(stdout);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
    disconnected = 1;
}

void onSubscribe(void* context, MQTTAsync_successData* response)
{
    subscribed = 1;
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    fprintf(stderr, "Subscribe failed, rc %s\n", MQTTAsync_strerror(response->code));
    finished = 1;
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    fprintf(stderr, "Connect failed, rc %s\n", response ? MQTTAsync_strerror(response->code) : "none");
    finished = 1;
}

void onConnect(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
    int rc;

printf("%s \n",__FUNCTION__);
    printf("Subscribing to topic %s with client %s at QoS %d\n", topic, clientid, qos);

    ropts.onSuccess = onSubscribe;
    ropts.onFailure = onSubscribeFailure;
    ropts.context = client;
    if ((rc = MQTTAsync_subscribe(client, topic, qos, &ropts)) != MQTTASYNC_SUCCESS)
    {
        fprintf(stderr, "Failed to start subscribe, return code %s\n", MQTTAsync_strerror(rc));
        finished = 1;
    }
}

MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;


int main(void)
{
    MQTTAsync client;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
    int rc = 0;

    rc = MQTTAsync_createWithOptions(&client, "222.112.107.46:1883", clientid, MQTTCLIENT_PERSISTENCE_NONE,
            NULL, &create_opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        fprintf(stderr, "Failed to create client, return code: %s\n", MQTTAsync_strerror(rc));
        exit(EXIT_FAILURE);
    }

    rc = MQTTAsync_setCallbacks(client, client, NULL, messageArrived, NULL);
    if (rc != MQTTASYNC_SUCCESS)
    {
        fprintf(stderr, "Failed to set callbacks, return code: %s\n", MQTTAsync_strerror(rc));
        exit(EXIT_FAILURE);
    }

    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.cleansession = 1;
    conn_opts.keepAliveInterval = keepalive;
    conn_opts.MQTTVersion = MQTTVersion;
    conn_opts.context = client;
    conn_opts.automaticReconnect = 1;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        fprintf(stderr, "Failed to start connect, return code %s\n", MQTTAsync_strerror(rc));
        exit(EXIT_FAILURE);
    }
    while (!subscribed) usleep(100 * 1000);

    if (finished)
        goto exit;

    while (!finished) usleep(100 * 1000);

    disc_opts.onSuccess = onDisconnect;
    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
    {
        fprintf(stderr, "Failed to start disconnect, return code: %s\n", MQTTAsync_strerror(rc));
        exit(EXIT_FAILURE);
    }
    while (!disconnected) usleep(100 * 1000);

exit:
    MQTTAsync_destroy(&client);
    return EXIT_SUCCESS;
}
