#pragma once

void Sys_Shutdown(void);
void Sys_SendKeyEvents(void);
int Sys_FileOpenRead(char *, int *);
int Sys_FileOpenWrite(char *);
void Sys_FileClose(int);
void Sys_FileSeek(int, int);
int Sys_FileRead(int, void *, int);
int Sys_FileWrite(int, void *, int);
