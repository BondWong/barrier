<h1>Barrier Synchronization</h1>
* * *
In this project, you will evaluate several barrier synchronization algorithms presented in [“Algorithms for Scalable Synchronization on Shared-Memory Multiprocessors” by Mellor-Crummey and Scott.](http://www.cs.rice.edu/~johnmc/papers/tocs91.pdf). To do this, you will implement some of them using OpenMP and MPI, standard frameworks for parallel programming in shared memory and distributed architectures, and then evaluate some experimental results. 

* * *
## Setup

1. Gain access to a well-maintained linux machine.  If you do not have this already, then it is recommended that you follow the [instructions](https://www.udacity.com/wiki/ud156-virtualbox) for downloading and installing VirtualBox and the AOS Virtual Image. (You can use VMware too.)
 
2. Download the starter source code by cloning it from this repository. 
3. Check that OpenMP is installed with

    ldconfig -p | grep libgomp
This should show the location fo the libgomp library.
4. Check that MPI is installed by confirming that mpicc and mpirun are in your path with

    which mpicc mpirun

* * *
## Collaboration
For this and all projects, students must work on their own.

* * *
## Programming for a Shared Memory Multiprocessor with OpenMP
OpenMP uses \#pragma directives to direct the compiler to create a multithreaded program.  A simple OpenMP program using barrier would look like this

    int main(int argc, char** argv){
      int num_threads = 5;
    
      /* Prevents runtime from adjusting the number of threads. */
      omp_set_dynamic(0);
      
      /* Making sure that it worked. */
      if (omp_get_dynamic())
        printf("Warning: dynamic adjustment of threads has been set\n");
    
      /* Setting number of threads */
      omp_set_num_threads(num_threads);
      
      /* Code in the block following the #pragma directive is run in parallel */
    #pragma omp parallel 
       {
    
         /*Some code before barrier ..... */
    
         /* The barrier*/
         #pragma omp barrier
    
         /*Some code after the barrier..... */
    
       }
    
       return(0);
    }

To replace OpenMP’s barrier algorithm with our own, we define the following API:

    /*This function should be called before the parallel section of the code.  It performs all necessary memory allocation and initialization necessary for a barrier with num_threads threads.*/
    gtmp_init(int num_threads) - 
    
    /* This function causes the calling thread to wait until all other threads have reached the same barrier. */
    gtmp_barrier() 
    
    /* This function performs any cleanup needed for the barrier. */
    gtmp_finalize() 

The above OpenMP program becomes the following when modified to use the gtmp barrier system

    int main(int argc, char** argv){
      int num_threads = 5;
    
      /* Prevents runtime from adjusting the number of threads. */
      omp_set_dynamic(0);
      
      /* Making sure that it worked. */
      if (omp_get_dynamic())
        printf("Warning: dynamic adjustment of threads has been set\n");
    
      /* Setting number of threads */
      omp_set_num_threads(num_threads);
      
    gtmp_init(num_threads);
      /* Code in the block following the #pragma directive is run in parallel */
    #pragma omp parallel 
       {
    
         /*Some code before barrier ..... */
    
         /* The barrier, instead of #pragma barrier*/
         gtmp_barrier();
    
         /*Some code after the barrier..... */
    
       }
    
      gtmp_finalize();
    
       return(0);
    }

The code for the project uses OpenMP for the shared memory experiments, but the code on which you will be evaluated can be written entirely in C.  You may find it useful, however, to write some test code using OpenMP to help you debug your program.  This is also an opportunity to learn about OpenMP more broadly.  Here are some good resources.

* [Wikipedia on OpenMP](http://en.wikipedia.org/wiki/OpenMP)
* [Official OpenMP site](https://www.openmp.org/)
* [Using OpenMP](http://mitpress.mit.edu/books/using-openmp) and [Examples (zip file)](https://openmp.org/wp-content/uploads/Using-OpenMP-Examples-Distr.zip)
* [Atomic Built-ins](http://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Atomic-Builtins.html)

For sample code, check out [hello_openmp.c](https://github.gatech.edu/smathew60/oms-aos-barrier/blob/master/hello_openmp.c)

* * *
## Distributed Memory with MPI
Although Mellor-Crummey and Scott describe algorithms for a shared memory environment, it is straightforward to adapt them to a distributed one.  Instead of spinning on a variable waiting for its value to be changed, a thread waits for a message from another thread.

The standard framework for message passing in high performance parallel computing is MPI.  More specifically, we will use the openmpi implementation of MPI-2.  A simple MPI program using barrier would look like this 


    int main(int argc, char** argv)
    {
      int req_processes = 5;
      int num_processes;
    
      MPI_Init(&argc, &argv);
      
      /* Start of Parallel...*/
        
      /* Making sure that we got the correct number of processes */
      MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
      if(num_processes != req_processes){
        fprintf(stderr, "Wrong number of processes.  Required: %d, Actual: %d\n",
    	    req_processes, num_processes);
        exit(1);
      }
      
    /* Code before barrier ...*/
    
      /* The barrier */
      MPI_Barrier(MPI_COMM_WORLD);
        
    /* Code after barrier ...*/
    
      /* Cleanup */
      MPI_Finalize();
    
      return(0);
    }

 
The API for your barrier system will be the same as for the first part of the project with the functions  gtmpi_init(int num_threads), gtmpi_barrier(), and gtmpi_finalize() all having the same semantics.

Replacing the built-in MPI barrier with the gtmpi version, we have 

    int main(int argc, char** argv)
    {
      int req_processes = 5;
      int num_processes;
    
      gtmpi_init(req_processes);
      MPI_Init(&argc, &argv);
      
      /* Start of Parallel...*/
        
      /* Making sure that we got the correct number of processes */
      MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
      if(num_processes != req_processes){
        fprintf(stderr, "Wrong number of processes.  Required: %d, Actual: %d\n",
    	    req_processes, num_processes);
        exit(1);
      }
      
    /* Code before barrier ...*/
    
      /* The barrier */
      gtmpi_barrier();
        
    /* Code after barrier ...*/
    
      /* Cleanup */
      MPI_Finalize();
      gtmpi_finalize();  
    
      return(0);
    }


As you program, think about which messages passing functions should be synchronous and which asynchronous.  Some useful documentation on MPI can be found in these resources:

* [Wikipedia on MPI](http://en.wikipedia.org/wiki/Message_Passing_Interface)
* [Tutorial](https://computing.llnl.gov/tutorials/mpi/)

Note that your should only use MPI's point-to-point communication procedures (i.e. MPI_Isend, MPI_Irecv, MPI_Send, MPI_Recv, NOT MPI_BCast, MPI_Gather, etc.)

For sample code, check out [hello_mpi.c](https://github.gatech.edu/smathew60/oms-aos-barrier/blob/master/hello_mpi.c)

---
## Tasks
Your tasks are the following:

1. Complete the implementation of the dissemination algorithm in gtmpi_dissemination.c.
2. Complete the implementation of the tournament algorithm in gtmpi_tournament.c.
3. Improve the implementation of the counter in gtmpi_counter.c.
4. Reply to the email Re:_MPI_Barrier.txt. (Add your insights in the same file)
    - Analyze GTMPI_Data.csv and provide your insights.
5. Complete the implementation of the counter barrier in gtmp_counter.c
6. Complete the implementation of the MCS tree in gtmp_mcs.c
7. Improve the performance of the tree barrier implementation in gtmp_tree.c and explain why your implementation will run faster in the comments.
8. Reply to the email in the file Re:_OpenMP_Barrier.txt. (Add your insights in the same file)
    - Analyze GTMP_Data.csv and provide your insights.

## Deliverables

Your submission should have the following eight files. Do not change the name of the files.
-	 gtmp_counter.c
-	 gtmp_mcs.c
-	 gtmp_tree.c
-	 Re:_OpenMP_Barrier.txt
-	 gtmpi_dissemination.c
-	 gtmpi_tournament.c
-	 gtmpi_counter.c
-	 Re:_MPI_Barrier.txt

Place all the eight files into a folder named FirstName_LastName_p2, and zip it. Zip file name should be the same as the folder name.

## Grading

<table>
<tr><td><b>Deliverable</b></td><td><b>Percentage</b></td></tr>
<tr><td>gtmp_counter.c</td><td>15%</td></tr>
<tr><td>gtmp_mcs.c</td><td>15%</td></tr>
<tr><td>gtmp_tree.c</td><td>10%</td></tr>
<tr><td>Re:_OpenMP_Barrier.txt</td><td>10%</td></tr>
<tr><td>gtmpi_dissemination.c</td><td>15%</td></tr>
<tr><td>gtmpi_tournament.c</td><td>15%</td></tr>
<tr><td>gtmpi_counter.c</td><td>10%</td></tr>
<tr><td>Re:_MPI_Barrier.txt</td><td>10%</td></tr>
</table>




## FAQs

-	What are the csv files?  It sounds like they are part of the email and we should respond based on those results?  Do we not use our own results to respond?

	Csv files provide data that must be used to analyse and answer parts Re:_OpenMP_Barrier.txt and Re:_MPI_Barrier.txt. You need to make your analysis based on this data, and not on your own results.

-	What are the gnu files?

	Gnu files help plot these csv files.

-	What is Network.txt and killerbee3_cpuinfo.txt for?

	Use this information for analysis/reasoning of csv data if required.

- What are the hello files for?

	Hello files are simple examples to help you get started on Open_MP and OpenMPI.

- Each of the algorithm files are just init, barrier, and finalize.  Do these files get called or included somehow?  Are we supposed to add a main? Are we supposed to make use of the provided Makefile?  Do we edit it?

	You need to edit the Makefile, to compile your barrier functions. You are supposed to test the functionality and correctness of these barriers on your own. There is no need to provide main function because we will be using our own test harness. Students have to supply only the functions in the requested files. Your code is written as a library, but you have to write separate code with mains that will make use of your library.  This separate code is not passed in.

-	Can you release your test harness?

	We can not release our test harness. You need to come up with your own test harness. If you want, you can share your harness with other students.

-	For project 2, are we required to follow the exact pseudocode from the paper? 

	As long as you don't deviate from how an algorithm is supposed to work, you should be fine!

-	Can we use sched_yield to improve performance?

	Yes you can. Just edit the makefile accordingly.

- Are we allowed to use the OMP threadprivate declaration?
	Yes, you can.

-	When I attempt to run the gtmp_tree algorithm using the test harness I am getting a heap bufferflow out of the box without any modifications. Is this expected?

	The error is because the algorithm doesn't work for 1 thread.
 
-	I am facing errors as in I encounter a problem where the -np options just don't seem to work. What is wrong?

	Environment variables aren't being set up properly. Follow the commands:

	>sudo apt remove libopenmpi-dev openmpi-bin
sudo apt purge mpich
sudo apt install mpich

-	Are we allowed to use code or function snippets from the internet?

	Cite, cite and cite! As long as your reference isn't giving out the explicit solution to what's asked in the assignment, you should be fine.

-	For both *_Data.csv files, am I correct in assuming that the units are in micro seconds?

	No. The units are in seconds.

-	I am having trouble with plotting GNU files. What can I do?

	One of the ways is:
	
	Install the packages:

	>sudo apt-get install gnuplot
sudo apt-get install gnuplot-nox

	Edit the gnu files to generate a pdf, set the terminal and output towards the top of the file. Also, you can delete the set timefmt lines, if they are causing an error.

	>set terminal pdf
	set output "mp_plot.pdf"

	Run gnuplot to generate the pdf graph

	>gnuplot mp_plot.gnu

-	I don't understand what the parity in disseminated barrier does. From the pseudo code the barrier uses two sets of flags alternatively for consecutive barriers. Why it needs two sets of flags?

	If you are trying to implement with mpi I'd skip looking at parity and the flags and focus on what the lecture describes. The parity ensures that the sense variable is only changed every other barrier. Since the barriers are spinning on 2 different sets of variables on consecutive barriers, there is no interference between the two and the number of spin loops reduces by half. (If they spin on the same variable, they would have to spin on sense variable at every barrier which is avoided now).

-	Will tournament barrier algorithm work with processors that are not power of 2?

	Yes, The algorithm (as presented in the pseudocode) will work with numbers of processors that aren't a power of 2.

## Tips (from a previous student, Joves)

1. First, get yourself a Linux system.  Either bare metal or a VM.  The one you used for project 1 will work.  There's also a VM you can download and install in VirtualBox that is linked in the project readme.  I'm using my project 1 VM. If you set up your own environment, make sure you install the libraries needed.

2. Your tasks for MPI part of the project are:
	1.	Complete the implementation of the dissemination algorithm in gtmpi_dissemination.c.
	2.	Complete the implementation of the tournament algorithm in gtmpi_tournament.c.
	3.	Improve the implementation of the counter in gtmpi_counter.c.
	4.	Reply to the email Re:_MPI_Barrier.txt
	
3. Start with part 3.  It gives you a good overview of the MPI calls you will need, and it's working code to start.  Fixing this code will be simpler than starting parts 1 and 2 from scratch.

4. Parts 1 and 2.  Here is where things get interesting and you have to decide on what you want to do.  You can follow the pseudo code, but that will leave you with a lot of ugly code as you try and shoehorn distributed memory code into the shared memory algorithm.

5. From the project doc: Although Mellor-Crummey and Scott describe algorithms for a shared memory environment, it is straightforward to adapt them to a distributed one. Instead of spinning on a variable waiting for its value to be changed, a thread waits for a message from another thread.
The key here is that you want to use a MPI message call (MPI_Isend, MPI_Irecv, MPI_Send, MPI_Recv) instead of spinning.  The simple and ugly way to do this is to merely replace the spinning parts of the pseudocode with blocking receive calls.  You then change the sense flips with a send call.

6. But you can do better.  Use the pseudo code as reference but read the paper carefully.  Rewatch the barrier lectures and rethink the algorithm from a distributed memory point of view.  At what part does a node need to wait.  At what part does a node need to tell another that it's done. 

7. **Testing**: The example code that you see in the readme is a skeleton for a test harness.  It's a main function that calls your implementations of the algorithms.  It's helpful to understand how MPI works, but it's very basic.
