/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "fork.h"

using namespace std;

int doFork() {
    int cpuCount = sysconf(_SC_NPROCESSORS_ONLN);
    cout << " found " << cpuCount << " cpu(s); ";
    cout << MODE << endl << endl;
    int forkedCount = 0;
    for(int i = 0; i < cpuCount;i++) {
        pid_t pid = fork();
        if(pid == 0){
#ifdef DEBUG
            cout << "forked process; exiting loop cpu=" << cpu << endl;
#endif
            return i; // return cpu id
        }else {
            forkedCount++;
        }
#ifdef DEBUG
        cout << "parent process: forking new one" << endl;
#endif
    }
    if(forkedCount == cpuCount) {
#ifdef DEBUG
       cout << "fork job done; exiting parent" << endl;
#endif
       pid_t wpid;
       int status = 0;
       while((wpid = wait(&status)) > 0 );
       exit(0);
    }
    return -1; //should never happen
}