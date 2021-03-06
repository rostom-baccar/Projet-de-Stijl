/*
 * Copyright (C) 2018 dimercur
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tasks.h"
#include <stdexcept>

// Déclaration des priorités des taches
#define PRIORITY_TSERVER 30
#define PRIORITY_TOPENCOMROBOT 20
#define PRIORITY_TMOVE 20
#define PRIORITY_TSENDTOMON 22
#define PRIORITY_TRECEIVEFROMMON 25
#define PRIORITY_TSTARTROBOT 20
#define PRIORITY_TCAMERA 21
#define PRIORITY_TCHECKBATTERY 20
#define PRIORITY_TWATCHDOGRELOAD 21

/*
 * Some remarks:
 * 1- This program is mostly a template. It shows you how to create tasks, semaphore
 *   message queues, mutex ... and how to use them
 * 
 * 2- semDumber is, as name say, useless. Its goal is only to show you how to use semaphore
 * 
 * 3- Data flow is probably not optimal
 * 
 * 4- Take into account that ComRobot::Write will block your task when serial buffer is full,
 *   time for internal buffer to flush
 * 
 * 5- Same behavior existe for ComMonitor::Write !
 * 
 * 6- When you want to write something in terminal, use cout and terminate with endl and flush
 * 
 * 7- Good luck !
 */

/**
 * @brief Initialisation des structures de l'application (tâches, mutex, 
 * semaphore, etc.)
 */
void Tasks::Init() {
    int status;
    int err;

    /**************************************************************************************/
    /* 	Mutex creation                                                                    */
    /**************************************************************************************/
    if (err = rt_mutex_create(&mutex_monitor, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_robot, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_robotStarted, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_move, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_watchdogMode, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
        if (err = rt_mutex_create(&mutex_robotErrors, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
        if (err = rt_mutex_create(&mutex_com_robot, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Mutexes created successfully" << endl << flush;

    /**************************************************************************************/
    /* 	Semaphors creation       							  */
    /**************************************************************************************/
    if (err = rt_sem_create(&sem_barrier, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_openComRobot, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_serverOk, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_startRobot, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_watchdogReload, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Semaphores created successfully" << endl << flush;

    /**************************************************************************************/
    /* Tasks creation                                                                     */
    /**************************************************************************************/
    if (err = rt_task_create(&th_server, "th_server", 0, PRIORITY_TSERVER, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_sendToMon, "th_sendToMon", 0, PRIORITY_TSENDTOMON, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_receiveFromMon, "th_receiveFromMon", 0, PRIORITY_TRECEIVEFROMMON, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_openComRobot, "th_openComRobot", 0, PRIORITY_TOPENCOMROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_startRobot, "th_startRobot", 0, PRIORITY_TSTARTROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_move, "th_move", 0, PRIORITY_TMOVE, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_checkBattery, "th_checkBattery", 0, PRIORITY_TCHECKBATTERY, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_watchdogReload, "th_watchdogUpdate", 0, PRIORITY_TWATCHDOGRELOAD, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Tasks created successfully" << endl << flush;

    /**************************************************************************************/
    /* Message queues creation                                                            */
    /**************************************************************************************/
    if ((err = rt_queue_create(&q_messageToMon, "q_messageToMon", sizeof (Message*)*50, Q_UNLIMITED, Q_FIFO)) < 0) {
        cerr << "Error msg queue create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Queues created successfully" << endl << flush;

}

/**
 * @brief Démarrage des tâches
 */
void Tasks::Run() {
    rt_task_set_priority(NULL, T_LOPRIO);
    int err;

    if (err = rt_task_start(&th_server, (void(*)(void*)) & Tasks::ServerTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_sendToMon, (void(*)(void*)) & Tasks::SendToMonTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_receiveFromMon, (void(*)(void*)) & Tasks::ReceiveFromMonTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_openComRobot, (void(*)(void*)) & Tasks::OpenComRobot, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_startRobot, (void(*)(void*)) & Tasks::StartRobotTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_move, (void(*)(void*)) & Tasks::MoveTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_checkBattery, (void(*)(void*)) & Tasks::CheckBattery, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_watchdogReload, (void(*)(void*)) & Tasks::WatchdogReload, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }

    cout << "Tasks launched" << endl << flush;
}

/**
 * @brief Arrêt des tâches
 */
void Tasks::Stop() {

    rt_mutex_acquire(&mutex_monitor, TM_INFINITE);
    monitor.Close();
    rt_mutex_release(&mutex_monitor);

    rt_mutex_acquire(&mutex_robot, TM_INFINITE);
    robot.Close();
    rt_mutex_release(&mutex_robot);

    rt_mutex_acquire(&mutex_com_robot, TM_INFINITE);
    comOK = false;
    rt_mutex_release(&mutex_com_robot);
}

/**
 */
void Tasks::Join() {
    //On réveille toutes les tâches en attente 
    cout << "Tasks synchronized" << endl << flush;
    rt_sem_broadcast(&sem_barrier);
    pause();
}

/**
 * @brief Thread handling server communication with the monitor.
 */
void Tasks::ServerTask(void *arg) {
    int status;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are started)
    rt_sem_p(&sem_barrier, TM_INFINITE);

    /**************************************************************************************/
    /* The task server starts here                                                        */
    /**************************************************************************************/
    rt_mutex_acquire(&mutex_monitor, TM_INFINITE);
    status = monitor.Open(SERVER_PORT);
    rt_mutex_release(&mutex_monitor);

    cout << "Open server on port " << (SERVER_PORT) << " (" << status << ")" << endl;

    if (status < 0) throw std::runtime_error {
        "Unable to start server on port " + std::to_string(SERVER_PORT)
    };
    monitor.AcceptClient(); // Wait the monitor client
    cout << "Rock'n'Roll baby, client accepted!" << endl << flush;
    rt_sem_broadcast(&sem_serverOk);
}

/**
 * @brief Thread sending data to monitor.
 */
void Tasks::SendToMonTask(void* arg) {
    Message *msg;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);

    /**************************************************************************************/
    /* The task sendToMon starts here                                                     */
    /**************************************************************************************/
    rt_sem_p(&sem_serverOk, TM_INFINITE);

    while (1) {
        cout << "wait msg to send" << endl << flush;
        msg = ReadInQueue(&q_messageToMon);
        
        cout << "Send msg to mon: " << msg->ToString() << endl << flush;
        rt_mutex_acquire(&mutex_monitor, TM_INFINITE);
        monitor.Write(msg); // The message is deleted with the Write
        rt_mutex_release(&mutex_monitor);
    }
}

/**
 * @brief Thread receiving data from monitor.
 */
void Tasks::ReceiveFromMonTask(void *arg) {
    Message *msgRcv;
    int status;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task receiveFromMon starts here                                                */
    /**************************************************************************************/
    rt_sem_p(&sem_serverOk, TM_INFINITE);
    cout << "Received message from monitor activated" << endl << flush;

    while (1) {
        msgRcv = monitor.Read();
        cout << "Rcv <= " << msgRcv->ToString() << endl << flush;

        if (msgRcv->CompareID(MESSAGE_MONITOR_LOST)) {
            
            //CONNECTION WITH MONITOR IS LOST [FUNCTION 5 & 7]
            cout << "Connection with monitor is lost" << endl << flush;
            
            //WE MAKE SURE THE ROBOT IS STARTED BEFORE STOPPING IT
            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
            int rs = robotStarted;
            rt_mutex_release(&mutex_robotStarted);
            
            if (rs == 1) {
            
            //STOP ROBOT [FUNCTION 6]        
            rt_mutex_acquire(&mutex_move, TM_INFINITE);
            move = MESSAGE_ROBOT_STOP;
            rt_mutex_release(&mutex_move);
            
            //CLOSE COMMUNICATION WITH ROBOT [FUNCTION 6]
            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            robotResponse = robot.Write(robot.PowerOff());
            robot.Close();
            rt_mutex_release(&mutex_robot);

            rt_mutex_acquire(&mutex_com_robot, TM_INFINITE);
            comOK = false;
            rt_mutex_release(&mutex_com_robot);


            //ROBOT ERRORS HANDLING
            if ((robotResponse->CompareID(MESSAGE_ANSWER_ROBOT_ERROR)) || 
                    (robotResponse->CompareID(MESSAGE_ANSWER_ROBOT_ERROR))) {
                rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                robotErrors++;
                rt_mutex_release(&mutex_robotErrors);
            }
            else {
                rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                robotErrors=0;
                rt_mutex_release(&mutex_robotErrors);
            }
            
            //ROBOT STARTED VARIABLE UPDATE [FUNCTION 6]
            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
            rs = 0;
            rt_mutex_release(&mutex_robotStarted);
            
            //STOP SERVER [FUNCTION 6]
            rt_mutex_acquire(&mutex_monitor, TM_INFINITE);
            monitor.Close();
            rt_mutex_release(&mutex_monitor);
            
            //ROBOT ERRORS RESET
            rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
            robotErrors=0;
            rt_mutex_release(&mutex_robotErrors);
            
            }
            
            delete(msgRcv);
            exit(-1);
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_COM_OPEN)) {
            
            //[FUNCTION 7]
            cout << "Communication with monitor OK" << endl << flush;
            cout << "Requesting communication with Robot" << endl << flush;
            
            rt_sem_v(&sem_openComRobot);            
            
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITHOUT_WD)) {
            cout << "Telling robot to start without watchdog" << endl << flush;
            rt_mutex_acquire(&mutex_watchdogMode, TM_INFINITE);
            watchdogMode = 0;
            rt_mutex_release(&mutex_watchdogMode);
            rt_sem_v(&sem_startRobot);
            
        //On rajoute la réception de message de démarrage avec watchdog
            
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITH_WD)) {
            cout << "Telling robot to start in watchdog mode" << endl << flush;
            rt_mutex_acquire(&mutex_watchdogMode, TM_INFINITE);
            watchdogMode = 1;
            rt_mutex_release(&mutex_watchdogMode);
            rt_sem_v(&sem_startRobot);
        } 
        
        else if (msgRcv->CompareID(MESSAGE_ROBOT_RESET)) {
            
            //WE MAKE SURE THE ROBOT IS STARTED
            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
            int rs = robotStarted;
            rt_mutex_release(&mutex_robotStarted);
            
            if (rs == 1) {
                
                //RESET ROBOT
                rt_mutex_acquire(&mutex_robot, TM_INFINITE);
                robotResponse = robot.Write(robot.Reset());
                rt_mutex_release(&mutex_robot);

                //ROBOT ERRORS HANDLING
                if (!robotResponse->CompareID(MESSAGE_ANSWER_ACK))  {
                    rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                    robotErrors++;
                    rt_mutex_release(&mutex_robotErrors);
                }
                else {
                    rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                    robotErrors=0;
                    rt_mutex_release(&mutex_robotErrors);
                }
            }
        }
        
        else if (msgRcv->CompareID(MESSAGE_ROBOT_GO_FORWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_BACKWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_LEFT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_RIGHT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_STOP)) {

            cout << "Recieved move request" << endl << flush;
            rt_mutex_acquire(&mutex_move, TM_INFINITE);
            move = msgRcv->GetID();
            rt_mutex_release(&mutex_move);
        }
        
        //ALL MESSAGES ARE RECIEVED AND TREATED [FUNCTION 3]
        
        else if (msgRcv->CompareID(MESSAGE_ROBOT_COM_CLOSE)){
            
        //CLOSING COMMUNICATION WITH ROBOT
        cout << "Requesting closing of communication with Robot" << endl << flush;
        rt_mutex_acquire(&mutex_robot, TM_INFINITE);
        status = robot.Close();
        rt_mutex_release(&mutex_robot);

        rt_mutex_acquire(&mutex_com_robot, TM_INFINITE);
        comOK = false;
        rt_mutex_release(&mutex_com_robot);

        
        //ROBOT ERRORS RESET
        rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
        robotErrors=0;
        rt_mutex_release(&mutex_robotErrors);
        }
        
        else if (msgRcv->CompareID(MESSAGE_ANSWER_ROBOT_UNKNOWN_COMMAND)){
            cout << "Request unknown" << endl << flush;
        }

        delete(msgRcv); // mus be deleted manually, no consumer
    }
}

/**
 * @brief Thread opening communication with the robot.
 */
void Tasks::OpenComRobot(void *arg) {
    int status;
    int err;

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task openComRobot starts here                                                  */
    /**************************************************************************************/
    while (1) {
        rt_sem_p(&sem_openComRobot, TM_INFINITE);

        cout << "Open serial com (";
        rt_mutex_acquire(&mutex_robot, TM_INFINITE);
        status = robot.Open();
        rt_mutex_release(&mutex_robot);
        cout << status;
        cout << ")" << endl << flush;

        Message * msgSend;
        if (status < 0) {
            msgSend = new Message(MESSAGE_ANSWER_NACK);
            cout << endl << flush;
            cout << "Connection to Robot unsuccessful" << endl << flush;

        } else {
            msgSend = new Message(MESSAGE_ANSWER_ACK);
            cout << "Connection to Robot successful" << endl << flush;
            comOK = true; //COMM WITH ROBOT ESTABLISHED SUCCESSFULLY
        }
        WriteInQueue(&q_messageToMon, msgSend); // msgSend will be deleted by sendToMon
    }

    
}

/**
 * @brief Thread starting the communication with the robot.
 */
void Tasks::StartRobotTask(void *arg) {
    //On distingue les deux modes de démarrage: sans watchdog et avec watchdog
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task startRobot starts here                                                    */
    /**************************************************************************************/
    while (1) {

        Message * msgSend;
        rt_sem_p(&sem_startRobot, TM_INFINITE);
        int wm; //watchdog
        bool comRobotOK=false;

        //WE MAKE SURE THE COMMUNICATION WITH THE ROBOT IS ESTABLISHED BEFORE STARTING THE ROBOT
        rt_mutex_acquire(&mutex_com_robot, TM_INFINITE);
        comRobotOK=comOK;
        rt_mutex_release(&mutex_com_robot);

        if (comRobotOK) {

            rt_mutex_acquire(&mutex_watchdogMode, TM_INFINITE);
            wm = watchdogMode; //selon le msg réceptionné depuis le moniteur
            rt_mutex_release(&mutex_watchdogMode);

            cout << "Watchdog Mode Selected = " << wm << endl << flush;
        
            if (wm==0){
        
                cout << "Start robot without watchdog (" << endl << flush;
                rt_mutex_acquire(&mutex_robot, TM_INFINITE);
                msgSend = robot.Write(robot.StartWithoutWD());
                rt_mutex_release(&mutex_robot);
 
                //ROBOT ERRORS HANDLING
                if (!msgSend->CompareID(MESSAGE_ANSWER_ACK))  {
                    rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                    robotErrors++;
                    rt_mutex_release(&mutex_robotErrors);
                }
                else {
                    rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                    robotErrors=0;
                    rt_mutex_release(&mutex_robotErrors);
                }
        
            }
            else if (wm==1){
        
                cout << "Start robot with watchdog (" << endl << flush;

                rt_mutex_acquire(&mutex_robot, TM_INFINITE);
                msgSend = robot.Write(robot.StartWithWD());
                rt_mutex_release(&mutex_robot);

                //ROBOT ERRORS HANDLING
                if (!msgSend->CompareID(MESSAGE_ANSWER_ACK))  {
                    rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                    robotErrors++;
                    rt_mutex_release(&mutex_robotErrors);
                }
                else {
                    rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                    robotErrors=0;
                    rt_mutex_release(&mutex_robotErrors);
                }
        
                //The watchdog reload semaphore is released here, the watchdog reload task can be executed
            rt_sem_v(&sem_watchdogReload);   
            }
        
        
            cout << msgSend->GetID();
            cout << ")" << endl;

            cout << "Movement answer: " << msgSend->ToString() << endl << flush;
            WriteInQueue(&q_messageToMon, msgSend);  // msgSend will be deleted by sendToMon

            if (msgSend->GetID() == MESSAGE_ANSWER_ACK) {
                rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
                robotStarted = 1;
                rt_mutex_release(&mutex_robotStarted);
            }
        }
        else cout << "Robot cannot be started: communication not established" << endl << flush;
    }
}

/**
 * @brief Thread handling control of the robot.
 */
void Tasks::MoveTask(void *arg) {
    int rs; 
    int cpMove;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task starts here                                                               */
    /**************************************************************************************/
    rt_task_set_periodic(NULL, TM_NOW, 100000000);

    while (1) {
        rt_task_wait_period(NULL);
        cout << "Periodic movement update" << endl << flush;
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        if (rs == 1) {
            rt_mutex_acquire(&mutex_move, TM_INFINITE);
            cpMove = move;
            rt_mutex_release(&mutex_move);
            
            cout << " move: " << cpMove;
            
            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            robotResponse = robot.Write(new Message((MessageID)cpMove));
            rt_mutex_release(&mutex_robot);

            //ROBOT ERRORS HANDLING
            if (!robotResponse->CompareID(MESSAGE_ANSWER_ACK))  {
                rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                robotErrors++;
                rt_mutex_release(&mutex_robotErrors);

            }
            else {
                rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                robotErrors=0;
                rt_mutex_release(&mutex_robotErrors);
            }
            
        }
        cout << endl << flush;
    }
}

/**
 * @brief Thread checking battery 
 */

void Tasks::CheckBattery(void *arg) {
    int rs;
    Message* batteryLevel; 

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task starts here                                                               */
    /**************************************************************************************/
    rt_task_set_periodic(NULL, TM_NOW, 500000000);

    while (1) {
        // Wait period
        rt_task_wait_period(NULL);
        cout << "Battery check" << endl << flush;
        
        // Lock mutex
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        // Unlock mutex
        rt_mutex_release(&mutex_robotStarted);
        
        if (rs == 1) { 
            // Get battery from robot 
            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            batteryLevel=robot.Write(robot.GetBattery());
            rt_mutex_release(&mutex_robot);
            
            cout << "Battery Level: " << batteryLevel->ToString() << endl << flush;
            // Display battery level on monitor - write to monitor 
            WriteInQueue(&q_messageToMon, batteryLevel);  
        }
        
        
        cout << endl << flush;
    }
}

    //Task qui met à jour le watchdog périodiquement
void Tasks::WatchdogReload(void *arg) {
    
    int rs;
    Message * watchdogReload;

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
   
    rt_task_set_periodic(NULL, TM_NOW, 1000000000);

    /**************************************************************************************/
    /* The task starts here                                                               */
    /**************************************************************************************/
    
    while(1){
        
        rt_task_wait_period(NULL);
        rt_sem_p(&sem_watchdogReload); 
        
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        
        if (rs == 1) { //We make sure the robot is started
            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            watchdogReload = robot.Write(robot.ReloadWD()); //the watchdog is reloaded
            rt_mutex_release(&mutex_robot);

            //ROBOT ERRORS HANDLING
            if (!watchdogReload->CompareID(MESSAGE_ANSWER_ACK)) {
                rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                robotErrors++;
                rt_mutex_release(&mutex_robotErrors);
            }
            else {
                rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
                robotErrors=0;
                rt_mutex_release(&mutex_robotErrors);
            }
                    
        }
    }     
}


    //[FUNCTION 8]
void Tasks::RobotErrorsHandling(void *arg) {

    int errors;

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);

    //100MS
    rt_task_set_periodic(NULL, TM_NOW, 100000000);

    /**************************************************************************************/
    /* The task starts here                                                               */
    /**************************************************************************************/
    
    while(1){
        
        rt_task_wait_period(NULL);
        
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        int rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        
        if (rs == 1) { //We make sure the robot is started
            
            rt_mutex_acquire(&mutex_robotErrors, TM_INFINITE);
            errors=robotErrors;
            rt_mutex_release(&mutex_robotErrors);

            if (errors>=3) {
                
                //COMM WITH ROBOT CLOSED
                rt_mutex_acquire(&mutex_robot, TM_INFINITE);
                int status = robot.Close();
                rt_mutex_release(&mutex_robot);

                rt_mutex_acquire(&mutex_com_robot, TM_INFINITE);
                comOK = false;
                rt_mutex_release(&mutex_com_robot);
            
            }
        }
    }     
}

/**
 * Write a message in a given queue
 * @param queue Queue identifier
 * @param msg Message to be stored
 */
void Tasks::WriteInQueue(RT_QUEUE *queue, Message *msg) {
    int err;
    if ((err = rt_queue_write(queue, (const void *) &msg, sizeof ((const void *) &msg), Q_NORMAL)) < 0) {
        cerr << "Write in queue failed: " << strerror(-err) << endl << flush;
        throw std::runtime_error{"Error in write in queue"};
    }
}

/**
 * Read a message from a given queue, block if empty
 * @param queue Queue identifier
 * @return Message read
 */
Message *Tasks::ReadInQueue(RT_QUEUE *queue) {
    int err;
    Message *msg;

    if ((err = rt_queue_read(queue, &msg, sizeof ((void*) &msg), TM_INFINITE)) < 0) {
        cout << "Read in queue failed: " << strerror(-err) << endl << flush;
        throw std::runtime_error{"Error in read in queue"};
    }/** else {
        cout << "@msg :" << msg << endl << flush;
    } /**/

    return msg;
}

