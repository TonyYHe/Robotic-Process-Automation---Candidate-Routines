# Robotic Process Automation---Candidate Routines 
Implmentations of simple mechanisms for checking candidate routines in a given trace of executed activities\
Project based on COMP10002 2020 Semester 2 Assignment 2\

## Stage 0 - Reading and Analayzing Input Data
read input data from stdin, ensure that the input trace is valid, and print out some basic information:
- number of distinct actions
- length of the input trace
- trace status
- the state of boolean variables after each valid action 

## Stage 1 - Check Basic Routines
Read candidate routines and output subsequence of the trace which produce the same cumulative effect of as the candidate routine.
Proceed from left to right in the trace, the shortest subsequence with the desired effect is identified. Once verified, the subsequence is to be recorded, 
and search restarts from the next position in the trace.

## Stage 2 - Check Advanced Routines 
Same procedure as stage 1, except that values of variables not set by the candidate routine are allowed to be modified. However, once all the actions of 
the identified subsequence are executed, the values of such variables must be set to the values they had before executing the subsequence.

