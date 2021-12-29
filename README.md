# Sorting-Visualizer-with-c
![bubble-sort](https://user-images.githubusercontent.com/31298786/147676686-41a9b11f-cafd-441c-9af6-6fa2a8b2f48f.gif)

# Description
The sorting visualizer is fully implemented with C. The sorting visualizer provides three different sorting methods including bubble sort, merge sort and quick sort. The user can custemize the visualizer with .srt file in given grammer. User can adjust the offset, color, height, and width of the rectanles shown. The number data to sort can also be set in the .srt file.

# Features
Recursive parser for .srt setting file, sorting algorithm, testing strategy

# How to use
The sample input .srt data is in the "SortData" folder. <br />
To make the sortingViz, type command "make sortingViz" <br />
To run the program, type command "./sortingViz SortData/filename.srt" <br />

# .srt Grammer
\<MAIN> ::= "{" <INSTRCTLST> <br />
\<INSTRCTLST> ::= <INSTRUCTION><INSTRCTLST> | "}" <br />
\<INSTRUCTION> ::= <SET> | <SORT> <br />
\<SET> ::= "SET" "{" <SETINSTRCTLST> <br />
\<SETINSTRCTLST> ::= <SETINSTRCT><SETINSTRCTLST> | "}" <br />
\<SETINSTRCT> ::= <HEIGHT> | <WIDTH> | <COLOR> | <TYPE> | <X> | <Y> | <DELAY> <br />
\<HEIGHT> ::= "HEIGHT" <NUM>   // set the height of the data unit of bar chart  <br />
\<WIDTH> ::= "WIDTH" <NUM>   // set the width of the bar in bar chart  <br />
\<COLOR> ::= "COLOR" <enum_COLOR>   // set the drawing color of the bar chart  <br />
\<TYPE> ::= "TYPE" <enum_TYPE>   // set the sorting type  <br />
\<X> ::= "X" <NUM>   // set the X offset of the entired bar chart  <br />
\<Y> ::= "Y" <NUM>   // set the Y offset of the entired bar chart  <br />
\<DELAY> ::= "DELAY" <NUM>   // set the time delay between each step of sorting  <br />
\<enum_COLOR> ::= "WHITE" | "RED" | "GREEN" | "BLUE" <br />
\<enum_TYPE> ::= "BUBBLE" | "MERGE" | "QUICK" <br />
\<SORT> ::= "SORT" "{" <NUMLST> <br />
\<NUMLST> ::= <NUM><NUMLST> | "}" <br />
\<NUM> ::= integer number <br />
  
# Testing strategy
The test of this program is performed through the _test(). <br />
The parameter settings and sorting functions are all tested. <br />
The error in the input data are tested by the error_code updated during the parsing. <br />

error_code listed below: <br />

Parse_OK : The input file is conformed to the grammar  <br />
error_BEGIN : The "{" as the entry of the program is missing <br />
error_INSTRUCTION : The <INSTRUCTION> input is wrong <br />
error_SET : There is no "{" after "SET"  <br />
error_RBRACE : The amount of "}" is wrong <br />
error_SETINSTRCT : The <SETINSTRCT> in the seting part is wrong <br />
error_COLOR : The input <enum_COLOR> is not defined in the tool <br />
error_TYPE : The input <enum_TYPE> is not defined in the tool <br />
error_SORT : There is no "{" after "SORT" <br />
error_NUM : The input is not a number which can be stored in integer <br />
error_CNUM : The amount of the input number exceed the maximun setting of the tool  <br />

# Screen recordings
## bubble sort
![bubble-sort](https://user-images.githubusercontent.com/31298786/147676686-41a9b11f-cafd-441c-9af6-6fa2a8b2f48f.gif)
  
## quick sort
![quick-sort](https://user-images.githubusercontent.com/31298786/147678853-460a6c04-48de-49b2-aee3-a73ae84898d9.gif)

## merge sort
![merge_sort](https://user-images.githubusercontent.com/31298786/147679490-fd1e9c11-a66f-42db-b9cf-cb69fc95a767.gif)

  
