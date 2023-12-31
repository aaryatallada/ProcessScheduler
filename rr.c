#include <fcntl.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <unistd.h>

/* A process table entry.  */
struct process
{
  long pid;
  long arrival_time;
    long burst_time;
    long og_burst;

  TAILQ_ENTRY (process) pointers;

  /* Additional fields here */
    bool done;
    long rtime;
    long wtime;
    bool active;
    long etime;
    long endtime;
  /* End of "Additional fields here" */
};

TAILQ_HEAD (process_list, process);

/* Skip past initial nondigits in *DATA, then scan an unsigned decimal
   integer and return its value.  Do not scan past DATA_END.  Return
   the integer’s value.  Report an error and exit if no integer is
   found, or if the integer overflows.  */
static long
next_int (char const **data, char const *data_end)
{
  long current = 0;
  bool int_start = false;
  char const *d;

  for (d = *data; d < data_end; d++)
    {
      char c = *d;
      if ('0' <= c && c <= '9')
    {
      int_start = true;
      if (ckd_mul (&current, current, 10)
          || ckd_add (&current, current, c - '0'))
        {
          fprintf (stderr, "integer overflow\n");
          exit (1);
        }
    }
      else if (int_start)
    break;
    }

  if (!int_start)
    {
      fprintf (stderr, "missing integer\n");
      exit (1);
    }

  *data = d;
  return current;
}

/* Return the first unsigned decimal integer scanned from DATA.
   Report an error and exit if no integer is found, or if it overflows.  */
static long
next_int_from_c_str (char const *data)
{
  return next_int (&data, strchr (data, 0));
}

/* A vector of processes of length NPROCESSES; the vector consists of
   PROCESS[0], ..., PROCESS[NPROCESSES - 1].  */
struct process_set
{
  long nprocesses;
  struct process *process;
};

/* Return a vector of processes scanned from the file named FILENAME.
   Report an error and exit on failure.  */
static struct process_set
init_processes (char const *filename)
{
  int fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      perror ("open");
      exit (1);
    }

  struct stat st;
  if (fstat (fd, &st) < 0)
    {
      perror ("stat");
      exit (1);
    }

  size_t size;
  if (ckd_add (&size, st.st_size, 0))
    {
      fprintf (stderr, "%s: file size out of range\n", filename);
      exit (1);
    }

  char *data_start = mmap (NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
    {
      perror ("mmap");
      exit (1);
    }

  char const *data_end = data_start + size;
  char const *data = data_start;

  long nprocesses = next_int (&data, data_end);
  if (nprocesses <= 0)
    {
      fprintf (stderr, "no processes\n");
      exit (1);
    }

  struct process *process = calloc (sizeof *process, nprocesses);
  if (!process)
    {
      perror ("calloc");
      exit (1);
    }

  for (long i = 0; i < nprocesses; i++)
    {
      process[i].pid = next_int (&data, data_end);
      process[i].arrival_time = next_int (&data, data_end);
      process[i].burst_time = next_int (&data, data_end);
        //ADDED
//      process[i].done = false;
//      process[i].rtime = 0;
//      process[i].wtime = 0;
      if (process[i].burst_time == 0)
    {
      fprintf (stderr, "process %ld has zero burst time\n",
           process[i].pid);
      exit (1);
    }
    }

  if (munmap (data_start, size) < 0)
    {
      perror ("munmap");
      exit (1);
    }
  if (close (fd) < 0)
    {
      perror ("close");
      exit (1);
    }
  return (struct process_set) {nprocesses, process};
}




int
main (int argc, char *argv[])
{
  if (argc != 3)
    {
      fprintf (stderr, "%s: usage: %s file quantum\n", argv[0], argv[0]);
      return 1;
    }

  struct process_set ps = init_processes (argv[1]);
  long quantum_length = (strcmp (argv[2], "median") == 0 ? -1
             : next_int_from_c_str (argv[2]));
  if (quantum_length == 0)
    {
      fprintf (stderr, "%s: zero quantum length\n", argv[0]);
      return 1;
    }

  struct process_list list;
  TAILQ_INIT (&list);

  long total_wait_time = 0;
  long total_response_time = 0;
    long cq = 0; //current quantum number
    struct process* cp = NULL; //current process pointer
    bool cs = false; //context switch
    bool medianquant = false;
  /* Your code here */
    if(quantum_length == -1){
        quantum_length = 1;
        medianquant = true;
    }
    //quantum length is -1 when argv2 is "median"
    long total_time = 0;
    struct process* iterator;
    //TODO: have to actually subtract from each process burst times (1)
    bool allProcessesDone = false;
    bool abtocum = false;
    bool justrem = false;
    for(int i = 0; i < ps.nprocesses; i++){
        ps.process[i].done = false;
        ps.process[i].rtime = 0;
        ps.process[i].wtime = 0;
        ps.process[i].active = false;
        ps.process[i].og_burst = ps.process[i].burst_time;
        ps.process[i].endtime = 0;

    }
    long arr[ps.nprocesses];
    for(int i = 0; i < ps.nprocesses; i++){
        arr[i] = 0;
    }
    int index = 0;
    long median = 1;
    while (!allProcessesDone)
    {

        justrem = false;
        abtocum = false;
        for(int i = 0; i < ps.nprocesses; i++){
            if(ps.process[i].arrival_time == total_time+1 && TAILQ_EMPTY(&list))
                abtocum = true;
        }
        
        if(cp != NULL && cp->burst_time == 0 ){
            cp->endtime = total_time;
            cq = quantum_length;
            //          cp = NULL;
        }
        if(cq == quantum_length){
            cs = !cs;
        }
        
        
        // ---------------------------------------MEDIAN----------------------------------------------------------------------------------------
        //IF !CS CALCULATE MEDIAN HERE. NOT CS BECAUSE DONT WANT TO MESS WITH CONTEXT SWITCHING.
        if (total_time == 0 && medianquant){
            cq = quantum_length;
        }
        index = 0;
        for(int i = 0; i < ps.nprocesses; i++){
            if(total_time >= ps.process[i].arrival_time){
                arr[index] = ps.process[i].og_burst - ps.process[i].burst_time;
                index++;
            }
        }
        arr[index] = -1;
        int j;
        for (j = 0; j < ps.nprocesses; j++){
            if(arr[j] == -1)
                break;
        }
        if(arr[0] == -1)
            median = 1;
        if(j == 1)
            median = arr[0];
        else if(j%2 == 0){ // even size
            median = (arr[(j/2) - 1] + arr[j/2])/2;
        }
        else{
            median = arr[j/2];
        }
        if(median == 0)
            median = 1;
        if(!cs && cq == quantum_length && medianquant){
            quantum_length = median;
            cq = 0;
            printf("NEW MEDIAN: %ld \n", median);
            for(int i = 0; i < j; i++){
                printf("CPU time exhausted  -- %ld \n" ,arr[i] );
            }
        }

            
        
        
        
        // ---------------------------------------MEDIAN----------------------------------------------------------------------------------------

        if(cs && cp->burst_time > 0 && TAILQ_EMPTY(&list) && !abtocum){
            continue;
        }
        if(abtocum)
            cs = false;
//        if(cs && TAILQ_EMPTY(&list) && cp == NULL) //if proc x finishes at end of 27 and proc y comes at 28 only one in q you wwant to cs still but not if theres nothing in queue
//            cs = false;

        if (cq == quantum_length || cp == NULL)
        {

            if(!cs)
                cq = 0;
            if(cs){
                if (cp != NULL && !cp->done)
                {
                    
                    // Put the current process back in the queue if it's not done
                    TAILQ_INSERT_TAIL(&list, cp, pointers);
                    cp = NULL;
                }

                // Get the next process from the queue
//                cp = TAILQ_FIRST(&list);
//                if (cp != NULL)
//                {
//                    TAILQ_REMOVE(&list, cp, pointers);
//                }
            }
        }

        for (int i = 0; i < ps.nprocesses; i++)
        {
            if (!ps.process[i].done && ps.process[i].arrival_time == total_time)
            {
                TAILQ_INSERT_TAIL(&list, &ps.process[i], pointers);
                //TODO: RESPONSE TIME BELOW IS WRONG
                if(ps.process[i].active){
                    ps.process[i].rtime = total_time - ps.process[i].burst_time - ps.process[i].wtime - ps.process[i].arrival_time;
                }
            }
        }
//MOVED THIS IF v ABOVE THE ONE BELOW (NOT RESPONSE TIME)
        if(cp != NULL && cp->burst_time == 0 && !cs){
            cp->endtime=total_time;
            cp = TAILQ_FIRST(&list);
            cp->active = true;
            if(cp != NULL){
                TAILQ_REMOVE(&list, cp, pointers);
                justrem = true;
            }
            

        }
//
//        for(int i = 0; i < ps.nprocesses; i++){
//            if(ps.process[i].active){
//                ps.process[i].rtime = total_time - ps.process[i].burst_time  - ps.process[i].arrival_time;
//            }
//        }

        
//        ///----------------_RESPONSE TIME_------------------------------------------------------------------------------------
        if(cp != NULL){
            if(cp->active == false){
                cp->active = true;
                cp->etime = total_time;
            }
        }
//        ///--------------------------------------------------------------------------


    if (cq == quantum_length || cp == NULL){
        if(cs){
            cp = TAILQ_FIRST(&list);
            if (cp != NULL)
            {
                TAILQ_REMOVE(&list, cp, pointers);
                justrem = true;
            }
        }
    }



//        if(total_time == 170)
//            allProcessesDone = true;
        // If current process pointer is NULL, try to fetch a process from the queue
        if (cp == NULL && !cs)
        {
            cp = TAILQ_FIRST(&list);
            if(cp != NULL){
                TAILQ_REMOVE(&list, cp, pointers);
                justrem = true;
            }
        }


        if (cp == NULL && !TAILQ_EMPTY(&list) && !cs)
        {
            cp = TAILQ_FIRST(&list);
            TAILQ_REMOVE(&list, cp, pointers);
            justrem = true;
        }

        if (cp != NULL && !cs)//IS THIS RIGHT !CS PART??
        {
            // Process is running
            cp->burst_time--;

            if (cp->burst_time == 0)
            {
                cp->endtime= total_time;
                // Process is done
                cp->done = true;
            }
        }



        // Increment wait time for all processes in the queue


        // Increment response time for processes that just started

        TAILQ_FOREACH(iterator, &list, pointers) {
            iterator->wtime++;
        }
        if(cs && cp != NULL){
            cp->wtime++;
        }
        if(cs && TAILQ_EMPTY(&list) && !abtocum && !justrem && cp != NULL){
            cp->wtime--;
            if(cp != NULL && cp->burst_time == 0){
                cp->endtime = total_time;
                cp = TAILQ_FIRST(&list);
                if(cp != NULL)
                    TAILQ_REMOVE(&list, cp, pointers);

            }
            if (cp == NULL)
            {
                cp = TAILQ_FIRST(&list);
                if(cp != NULL)
                    TAILQ_REMOVE(&list, cp, pointers);
            }
            if (cp == NULL && !TAILQ_EMPTY(&list))
            {
                cp = TAILQ_FIRST(&list);
                TAILQ_REMOVE(&list, cp, pointers);
            }
            cp->burst_time--;

            if (cp->burst_time == 0)
            {
                cp->endtime = total_time;
                // Process is done
                cp->done = true;
            }
        }


        // Check if all processes are done
        int counter = 0;
        for (int i = 0; i < ps.nprocesses; i++)
        {
            printf("TOTAL TIME %ld \n", total_time);
            printf("pid %ld, burst %ld, wait : %ld \n", ps.process[i].pid, ps.process[i].burst_time, ps.process[i].wtime);
            if (ps.process[i].done)
            {
                counter++;
            }
        }
        if (counter == ps.nprocesses && TAILQ_EMPTY(&list))
        {
            allProcessesDone = true;
        }
        total_time++;
        if(!cs){
            cq++;
        }
    }
//    for(int i = 0; i < ps.nprocesses; i++){
//        ps.process[i].wtime--;
//    }
    
    
    
    
    //-----------------------------__RESPONSE TIME___---------------------------------
//    for (int i = 0; i < ps.nprocesses; i++)
//    {
//        ps.process[i].rtime = ps.process[i].etime - ps.process[i].arrival_time;
//    }
    //-----------------------------__RESPONSE TIME___---------------------------------

    
    for(int i = 0; i < ps.nprocesses; i++){
        if(ps.process[i].active){
            ps.process[i].rtime = ps.process[i].etime - ps.process[i].burst_time  - ps.process[i].arrival_time;
            printf("pid: %ld and endtime: %ld\n", ps.process[i].pid, ps.process[i].endtime);
        }
    }
    
    if(medianquant){
        for(int i = 0; i < ps.nprocesses; i++){
            ps.process[i].endtime--;
            ps.process[i].wtime = ps.process[i].endtime - ps.process[i].arrival_time - ps.process[i].og_burst;
        }
    }
    
//    printf("done status of %ld is %d", ps.process[0].pid, ps.process[0].done);
//    printf("total time at the end is %d", total_time);
    // Calculate total wait and response times
    for (int i = 0; i < ps.nprocesses; i++)
    {
        total_wait_time += ps.process[i].wtime;
        total_response_time += ps.process[i].rtime;
    }
    
    total_response_time -= 1;

//big loop
    //if cq = qlength set cq to 0
    //if cq = 0
        //if nothing in queue and curr process still has more time to run then keep running cq++
        //else if nothing running run top of queue cq++
        //else context switch - add 1 to wait time for each proc in q add 1 to total time cq++. Also see if process just ran should be put back into queue if not (process is finished) set done boolean to true
        //
    //if currquant < qlength no ctxt switch
        //loop through queue 1 to wait time for each process
    // parse through and check if anything arrives. Add to queue.
    //if arrival time of any process(ties give priority to ordering in txt) = total time start running
    //add to rtime(response time) of process in the q
    

  /* End of "Your code here" */

    
    
    
    
  printf ("Average wait time: %.2f\n",
      total_wait_time / (double) ps.nprocesses);
  printf ("Average response time: %.2f\n",
      total_response_time / (double) ps.nprocesses);

  if (fflush (stdout) < 0 || ferror (stdout))
    {
      perror ("stdout");
      return 1;
    }

  free (ps.process);
  return 0;
}
