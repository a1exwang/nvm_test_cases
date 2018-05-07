int myomp;
 
#pragma omp threadprivate(myomp)
 
int main1() {
   #pragma omp parallel
   {
       myomp = 1;
   /*     printf("Thread x= %d\n", myomp); */
   }  

   return 0;
 }
