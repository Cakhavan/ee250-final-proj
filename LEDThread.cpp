/**
 * Copyright (c) 2017, Autonomous Networks Research Group. All rights reserved.
 * Developed by:
 * Autonomous Networks Research Group (ANRG)
 * University of Southern California
 * http://anrg.usc.edu/
 *
 * Contributors:
 * Jason A. Tran <jasontra@usc.edu>
 * Bhaskar Krishnamachari <bkrishna@usc.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * with the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * - Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimers.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *     this list of conditions and the following disclaimers in the 
 *     documentation and/or other materials provided with the distribution.
 * - Neither the names of Autonomous Networks Research Group, nor University of 
 *     Southern California, nor the names of its contributors may be used to 
 *     endorse or promote products derived from this Software without specific 
 *     prior written permission.
 * - A citation to the Autonomous Networks Research Group must be included in 
 *     any publications benefiting from the use of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH 
 * THE SOFTWARE.
 */

/**
 * @file       LEDThread.cpp
 * @brief      Implementation of thread that handles LED requests.
 *
 * @author     Jason Tran <jasontra@usc.edu>
 * @author     Bhaskar Krishnachari <bkrishna@usc.edu>
 */

#include "LEDThread.h"
#include "MQTTmbed.h"
#include "MQTTNetwork.h"

#include "MQTTClient.h"

Mail<MailMsg, LEDTHREAD_MAILBOX_SIZE> LEDMailbox;

extern void movement(char command, char speed, int delta_t);

static DigitalOut led2(LED2);

static const char *topic = "m3pi-mqtt-ee250/thread1";

int state;
int count=0;
float buf[125];
float avgbuf[25];

void LEDThread(void *args) 
{
    MQTT::Client<MQTTNetwork, Countdown> *client = (MQTT::Client<MQTTNetwork, Countdown> *)args;
    MailMsg *msg;
    MQTT::Message message;
    osEvent evt;
    char pub_buf[25];

    AnalogIn Ain(p20);
    float ADCdata;
    float distance;
    //state machine
    state=0;
    //buffer incrementer
    int a =0;
    //flag which data point was farthest
    int flag=0;
    int regress = 0;
    int tweak = 0;
    float max =0;
    int babe = 0;
     //scan
    printf("\nbegin\n");

    
    while(babe==0){
        evt = LEDMailbox.get();

        if(evt.status == osEventMail) {
msg = (MailMsg *)evt.value.p;
            if(msg->content[1] == 98){
            babe=1;
        }else if(msg->content[1] == 102){
            babe=2;
        }
    }
}
label_name:
    while(babe==1)
    {
        evt = LEDMailbox.get();

        if(evt.status == osEventMail) {
msg = (MailMsg *)evt.value.p;
            if(msg->content[1] == 98){
            babe=1;
        }else if(msg->content[1] == 102){
            babe=2;
        }

        }
                    pub_buf[0] = 's';
                    pub_buf[1] = 'c';
                    pub_buf[2] = 'a';
                    pub_buf[3] = 'n';
                    pub_buf[4] = 'n';
                    pub_buf[5] = 'i';
                    pub_buf[6] = 'n';
                    pub_buf[7] = 'g';

                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 10; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic, message);
                    mqttMtx.unlock();    
        
        if(state==0) {
            max=0;
            flag=0;
            a=0;
            regress = 0;
            while(count<=600)
            {
                //120 iterations
                count+=5; 

                //read ADC data
                ADCdata=Ain.read();

                //0.0032 V per cm
                //distance = cm
                distance = ADCdata * 2.38 / 0.0032; 

                //store data
                buf[a]=distance;
                printf("Distance of %d: %f\n", a, buf[a]);
                a++;

                
                //1 full turn
                movement('d',10,30);
                // if(buf[a]>45){
                //     movement('s',30,200);
                //     wait(.1)
                // }
            
                

            } /* while */

            printf("\ndone scanning\n"); 



            a=1;    
            max=0; 
            // store buf into an avgbuf 
            for(int i = 0; i < 25; i++)
            {
                for(int j = 0 ; j < 5 ;j++){
                   
                    avgbuf[i] += buf[a];
                    a++;
                }
                avgbuf[i] = avgbuf[i] / 5;
                printf("avg %d : %f",i,avgbuf[i]);
                pub_buf[i]= avgbuf[i];
                
            }

             /* for */

                    pub_buf[0] = 'a';
                    pub_buf[1] = 'v';
                    pub_buf[2] = 'e';
                    pub_buf[4] = 'r';
                    pub_buf[5] = 'a';
                    pub_buf[6] = 'g';
                    pub_buf[7] = 'i';
                    pub_buf[8] = 'n';
                    pub_buf[9] = 'g';
                   

                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 10; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic, message);
                    mqttMtx.unlock();
            //find max avg
            for(int i = 0; i < 25 ; i++){

                if(max<avgbuf[i]){
                    max = avgbuf[i];
                    flag = i;
                }

            }

            regress = count - (flag * 25);

            state=1;
            count=regress;

            // printf("\nThe data array is: \n");
            // for(int i=1; i<20; i++){
            //     printf("\nElement_%d: %f\n",i,buf[i]);
            // }
            // printf("\nthis is the max value: %f\n",max);

            // // mqttMtx.lock();
            //         client->publish(topic, max);
            // //         mqttMtx.unlock();
        }
                            
        if(state==1)
        {
            while(count>0){
            //wait(.5);
            
            movement('a',10,30);
        
            count-=5;
            //wait(.5);
             }
        }
            state=3;
        
        if(state==3)
        {
            count=0;
                     pub_buf[0] = 'm';
                     pub_buf[1] = 'o';
                     pub_buf[2] = 'v';
                     pub_buf[4] = 'i';
                     pub_buf[5] = 'n';
                     pub_buf[6] = 'g';
                     pub_buf[7] = ' ';
                     pub_buf[8] = 'f';
                     pub_buf[9] = 'o';
                    pub_buf[10] = 'r';
                    pub_buf[11] = 'w';
                    pub_buf[12] = 'a';
                    pub_buf[13] = 'r';
                    pub_buf[14] = 'd';

                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 15; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic, message);
                    mqttMtx.unlock();
            while(count<5)
            {
                movement('s',15,200);
                
                printf("or here\n");
                count++;
            }
                    
            state=0;
            
        }
        

    }


    //manual mode
    while(babe==2){


evt = LEDMailbox.get();

        if(evt.status == osEventMail) {
msg = (MailMsg *)evt.value.p;
            if(msg->content[1] == 98){
            babe=1;
        }else if(msg->content[1] == 99){ //forward
            movement('s',15,500);
        }else if(msg->content[1] == 100){ //turn left
            movement('a',15,200);
        }else if(msg->content[1] == 101){ //turn right
            movement('d',15,200);
        }else if(msg->content[1] == 103){ //scan
                    pub_buf[0] = 's';
                    pub_buf[1] = 'c';
                    pub_buf[2] = 'a';
                    pub_buf[3] = 'n';
                    pub_buf[4] = 'n';
                    pub_buf[5] = 'i';
                    pub_buf[6] = 'n';
                    pub_buf[7] = 'g';

                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 10; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic, message);
                    mqttMtx.unlock();   

                    count =0;
                    while(count<=600)
            {
                //120 iterations
                count+=5; 

                //read ADC data
                ADCdata=Ain.read();

                //0.0032 V per cm
                //distance = cm
                distance = ADCdata * 2.38 / 0.0032; 

                //store data
                buf[a]=distance;
                a++;

                
                //1 full turn
                movement('d',10,30);
            
                //wait(.1);

            } /* while */ 
        }

        }







    }//while2
goto label_name;
    while(1) {

        evt = LEDMailbox.get();

        if(evt.status == osEventMail) {
            msg = (MailMsg *)evt.value.p;
           
            /* the second byte in the message denotes the action type */
            switch (msg->content[1]) {
                case LED_THR_PUBLISH_MSG:
                    printf("LEDThread: received command to publish to topic"
                           "m3pi-mqtt-example/thread1\n");
                    pub_buf[0] = 'y';
                    pub_buf[1] = 'o';
                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 2; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic, message);
                    mqttMtx.unlock();
                    break;
                case LED_ON_ONE_SEC:
                    printf("LEDThread: received message to turn LED2 on for"
                           "one second...\n");
                    led2 = 1;
                    wait(1);
                    led2 = 0;
                    break;
                case LED_BLINK_FAST:
                    printf("LEDThread: received message to blink LED2 fast for"
                           "one second...\n");
                    for(int i = 0; i < 10; i++)
                    {
                        led2 = !led2;
                        wait(0.1);
                    }
                    led2 = 0;
                    break;
                default:
                    printf("LEDThread: invalid message\n");
                    break;
            }            

            LEDMailbox.free(msg);
        }
    } /* while */

    /* this should never be reached */
}

Mail<MailMsg, LEDTHREAD_MAILBOX_SIZE> *getLEDThreadMailbox() 
{
    return &LEDMailbox;
}


