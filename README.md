# Usage
___
1. Download the zip with all the student projects inside ./data/labX/
2. Create the rubric.txt and template_rubric.txt inside ./meta_data/labX/. It is also recommend copy the problem pdf and the data files for reference
2. Modify the list of names in the script 2-change_folder_name.ps1. Make sure any name has a tilde
3. Modify the list of names in the script 3-create_rubrics.ps1 with the same above list. Don't forget the tildes
4. Also modify the rubric template in 3-create_rubrics.ps1 at the final of the script. Make sure 
   the name of the studen and the word "Alumno:" stay in the same line.
5. Copy the scripts to ./data/labX and execute in order from 1 to 4. The 5 script is for the final step
6. Fill the rubric with each student
7. In main.cpp update the lab string value to match the ./data/labX folder and then execute it
8. Execute the script 5 in ./data/labX and copy into Paideia


The template for the 3-create_rubrics.ps1 must look like this
@"
Alumno: $studentLine
-
Puntaje
{Axel tiene que rellenar los criterios aqui}
-
Descuentos
a. [-3.00] 0.00
b. [-2.00] 0.00
c. [-0.50] 0.00
d. [-1.00] 0.00
e. [-0.25] 0.00
f. [-1.00] 0.00
-
Observaciones

"@
