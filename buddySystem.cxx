/*
 * File name: buddySystem.cxx
 * Author: Thomas Jarvinen
 * Classes:
 * 	Buddy: This class implements the buddy system memory allocation scheme as a tree
 * 		Member procedures:
 * 			Buddy(void * memBegin, int size)	Constructor. Creates a new block of buddy-system allocated memory
 * 			~Buddy(): Deletes all Buddy objects in the tree 
 * 			void * allocate(size_t, void *): Puts the data of length size_t stored at void* in the memory block.  Returns pointer to the location
 * 			Buddy* check(size_t): returns the leftmost terminal node of the tree that can hold a value of size_t
 * 			void merge(): Deletes the children of the calling node
 * 			void split(): Creates children for the calling node
 * 			void * getLocation: Returns the memory address stored in a node
 * 			size_t getMaxLength(): returns the memory capacity of the calling node
 * 			size_t getLength(): returns the length of memory in the node that is in use
 * 			Buddy* getLeft(): gets the left child of a node
 * 			Buddy* getRight(): gets the right  child of a node
 * 			bool isTerminal(): gets the terminal parameter of the node
 * 			void printTree(int): prints the amount of memory stored in each node of the tree
 * 			void setLength(size_t): length parameter, which holds the amound of memory in use in a node
 *	Other procedures:		
 * 		size_t sizeToBytes(int): Converts the height of the tree into capacity in bytes
 *		int bytesToSize(size_t): Given a number of bytes, returns the height of the tree needed to hold it
 * 
 * 			
 */


#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <iomanip>
#include <math.h>
#define MAX_MEMORY_OFFSET (1048576)	//Size of memory space: 1 MB
#define BASE_SIZE (65536)			//Minimum size of a memory block: 64 KB
using namespace std;
size_t sizeToBytes(int);
int bytesToSize(size_t);
/*
 * Class: Buddy
 * Author: Thomas Jarvinen
 * This class implements the buddy system memory allocation scheme as a tree.
 * In this scheme, memory is divided in blocks whose size are a power of 2.
 * Each block has a "buddy" which shares a size with it.  In essence, this means
 * the system is a binary tree.  This class services memory allocation and release requests
 * by recursively searching it's tree, finding the smallest appropriate unallocated block,
 * and then splitting it in half until wasted memory is minimized.
 * 
 * When a Buddy object is initialized, it takes two arguments to it's constuctor -- a pointer and
 * a size.  The pointer should point to a block which has been reserved with malloc.  The size
 * argument should indicate the amount of memory which has been allocated.
 * 
 * Class Variables:
 * 	void * memStart: the address of the beginning of the memory block belonging to the object.
 * 	size_t memLength: the amount of memory in the memory block which has been allocated.
 * 	int size:	Indicates the height of the tree (i.e. how many layers of children the class can have)
 * 	Buddy * left: pointer to left child
 * 	Buddy * right: pointer to right child
 * 	bool: terminal indicates whether the object has children
 * Class Methods: loosely defined above, defined in detail below
 */

class Buddy {
	void * memStart;
	size_t memLength;
	int size;	
	Buddy * left;	
	Buddy * right;
	bool terminal;
public:
	Buddy(void*,int);//done
	~Buddy();
	void * allocate(size_t,void *);
	bool release(void *);
	Buddy* check(size_t);
	void merge();//done
	void split();//done
	void * getLocation();//done
	size_t getMaxLength();//done
	size_t getLength();
	Buddy* getLeft();
	Buddy* getRight();
	bool isTerminal();
	void printTree(int);
	void setLength(size_t);
};
/*
 * Method: Buddy::Buddy()
 * Author: Thomas Jarvinen
 * Constructor for Buddy class, initializes variables
 * Arguments:
 * 	void * memBegin: input; used to initialize memStart
 * 	int width: input; used to initialize size
 * 	Other variables are initialized to NULL, 0, or false
 */
Buddy::Buddy(void * memBegin, int width)
{
	memStart = memBegin;//Start of memory block
	size  = width;		//Size of node
	left = NULL;		//No children yet
	right = NULL;		
	terminal = 0;		//Intuitively, this should be true, but I wrote this code backwards
	memLength = 0;		//No memory in use
}
/*
 * Method: Buddy::~Buddy()
 * Author: Thomas Jarvinen
 * Destuctor, recursively destroys all children
 * Arguments: None
 */
Buddy::~Buddy()
{
	if(left)delete left;	//Destroy left branch
	if(right)delete right;	//Destroy right branch
}
/*
 * Method: Buddy::allocate()
 * Author: Thomas Jarvinen
 * Tries to allocate a block of to hold the data of size numBytes that is contained in data
 * Arguments:
 * 	size_t numBytes: input length in bytes which is needed to store data
 * 	void * data: input; pointer to data to be copied
 * Output:
 * 	void * pointer to where the memory has been allocated.  If there is no spot for the data, NULL is returned
 */
void * Buddy::allocate(size_t numBytes, void * data)
{
	Buddy * location = this->check(numBytes);	//Get the leftmost terminal unallocated block which can hold data
	if(location == NULL)//There is no available location
	{
		return NULL;
	}
	//If we haven't returned yet, we now have a valid location

	while(location->getMaxLength() > numBytes*2 && bytesToSize(location->getMaxLength())>0)//We need to split until the space is small enough
	{
		location->split();		//Split to create children
		location = location->getLeft();	//The left child is our new location.  Choosing left over right is arbitrary
	}
	//location now points to the target memory location
	memcpy(location->getLocation(),data,numBytes);	//Copy data into location
	location->setLength(numBytes);					//Change the buddy memory variable to indicate how much data is being stored
	return location->getLocation();					//Return location of the data
}
/*
 * Method: Buddy::release()
 * Author: Thomas Jarvinen
 * This method recursively searches the tree for a memory allocation at location memToRelease.
 * It then releases the memory held there by changing memLength to 0 at the appropriate Buddy object.
 * Then, as the recursion unrolls, any appropriate merges are made to the tree.
 * Arguments:
 * 	void * memTorelease: input; indicates location of memory to be released
 * Output:
 * 	bool: indicates whether the deallocation was successful.  This will be true if memToRelease was a valid location
 * 
 */
bool Buddy::release(void * memToRelease)
{
	bool returnValue;
	if(!terminal)	//Either invalid location or we are at the correct location
	{
		if(memToRelease==memStart)//Correct location
		{
			memLength = 0;	//Deallocate
			returnValue = 1;		//Deallocation successful
		}
		else returnValue = 0;		//Invalid location, returnValue = false
	}
	else if(memToRelease == memStart)	//Valid location, we need to find terminal child
	{
		if(!left->isTerminal())	//The left child is terminal
		{
			left->setLength(0);		//release its memory
			returnValue = 1;		//We found the correct location, so returnValue = true

		}
		else returnValue =(left->release(memToRelease));	//left child is not terminal, recurse left
	}
	else if(memToRelease == right->getLocation())//The right child holds the correct location, need to find its terminal child
	{
		if(!right->isTerminal())	//right is terminal, release its memory
		{
			right->setLength(0);	//release its memory
			returnValue = 1;		//We found the correct location, so returnValue = true
		}
		else returnValue = right->release(memToRelease);
	}
	else  //Need to find location of memory, so recurse left, then if gives an invalid value, recurse right
	{
		if(left->release(memToRelease))returnValue = 1;		//Recurse Left
		else returnValue = right->release(memToRelease);	//If left failed to find, recurse right
	}
	if(terminal && right->getLength() == 0 && ! right->isTerminal() //Check for merge condition (i.e both left and right children are terminal and have no data
	&&	left->getLength() == 0 && !left->isTerminal()) this->merge(); //Since this method recursively searches the tree, it also automatically merges as needed
	return returnValue;												//Returning at the end of the method allows us to check for merge conditions only once.
}
/*
 * Method: Buddy::check()
 * Author: Thomas Jarvinen
 * Finds the smallest terminal unallocated memory block in the tree which cans contain numBytes
 * Arguments:
 * 	size_t numBytes: input, the minimum # of bytes of space needed
 * Output:
 * 	Buddy * pointer to the Buddy object for the memory block.  NULL is returned if none is found.
 */
Buddy* Buddy::check( size_t numBytes)
{
	int minSize = bytesToSize(numBytes);	//Minimum tree height
	if(size<minSize || memLength != 0)				//too small or already allocated
	{
		return NULL;						//There is no valid location
	}
	else if(!terminal)						//We are at a terminal branch that has not been allocated, so return
	{
		return this;
	}
	else if(left->check(numBytes) != NULL)	//Check Left child
	{
		return left->check(numBytes);		//Recurse left
	}
	else if(right->check(numBytes) != NULL)	//Check Right child
	{
		return right->check(numBytes);		//Recurse right
	}
	else
	{
		return NULL;						//Tree has no room
	}
}	
/*
 * Method: Buddy::merge()
 * Author: Thomas Jarvinen
 * Destroys a branch of a tree, merging it
 * Arguments:
 * 	None
 */
void Buddy::merge()
{
	delete left;	//Recursively delete left branch
	delete right;	//Recursively delete right branch
	terminal = 0;	//Indicate there are no children
	left = NULL;	//Get rid of hanging pointers
	right = NULL;	//Get rid of hanging pointers
}
/*
 * Method: Buddy::split()
 * Author: Thomas Jarvinen
 * Splits a node into a branch of the tree, by splitting the calling object's memory block in half,
 * and giving the first half to a newly created left child, and the second half to the right child
 * Arguments:	None
 */
void Buddy::split()
{
	left = new Buddy(memStart,size-1);	//Create a left child with half of the available memory block
	right = new Buddy(memStart+sizeToBytes(size-1),size-1);//Create a right child with the second half of the memory block
	terminal=1;	//The calling object is no longer a terminal leaf
}
/*
 * Method: Buddy::setLength()
 * Author: Thomas Jarvinen
 * 	Setter method for length
 */
void Buddy::setLength(size_t numBytes)
{
	memLength = numBytes;
}
/*
 * Method: Buddy::printTree()
 * Author: Thomas Jarvinen
 * Recursively prints the whole tree visible to the calling object
 * Arguments:
 * 	int width: input: give offset needed for templating
 * Output: prints memory allocation of each leaf of the tree to terminal
 */
void Buddy::printTree(int width = 0)
{
	if(width == 0)	//Before recursion, print helper text
	{
		//Helper text which indicates the size of each layer of the tree
	 	cout<<"Size: " << "4" <<setw(7)<< "3"<<setw(7) << "2"<<setw(7) << "1"<<setw(7) << "0" << endl;
	}
	if(left) left->printTree(width + 7);//Print left branch recursively
	if(width)
	{
		cout << setw(width) << ' ';	//Indent by width
	}
	cout << setw(7) << memLength << endl;	//Print memory used by the current node
	if(right) right->printTree(width+7);//Print right branch recursively
}
/*
 * Method: Buddy::setLength()
 * Author: Thomas Jarvinen
 * 	Getter method for location pointer
 */
void * Buddy::getLocation()
{
	return memStart;
}
/*
 * Method: Buddy::setLength()
 * Author: Thomas Jarvinen
 * 	getter method for memory capacity of a node
 */
size_t Buddy::getMaxLength()
{
	return sizeToBytes(size);
}
/*
 * Method: Buddy::setLength()
 * Author: Thomas Jarvinen
 * 	getter method for length
 */
size_t Buddy::getLength()
{
	return memLength;
}
/*
 * Method: Buddy::setLength()
 * Author: Thomas Jarvinen
 * 	Getter method for left child
 */
Buddy* Buddy::getLeft()
{
	return left;
}
/*
 * Method: Buddy::setLength()
 * Author: Thomas Jarvinen
 * 	Getter method for right child
 */
Buddy* Buddy::getRight()
{
	return right;
}
/*
 * Method: Buddy::setLength()
 * Author: Thomas Jarvinen
 * 	Getter method for terminal
 */
bool Buddy::isTerminal()
{
	return terminal;
}
/*
 * Method: main()
 * Author: Thomas Jarvinen
 * 	This method demonstrates the functionality of the Buddy class through an interactive
 * 	terminal interface for allocationa and deallocation 
 * allocations and deallocations
 * Arguments: none
 */
int main(void)
{
	void * memoryBlock = malloc(MAX_MEMORY_OFFSET);	//Allocate memory which will be reserved for our Buddy 
	Buddy* head = new Buddy(memoryBlock, 4);		//Create a new Buddy tree with max depth 4
	void * test;									//Pointer for demonstration data allocation calls
	int i,j,k;										//Helper variables for loops
	void * memLocations[16];						//A void * array which will hold pointers to the memory locations that have been allocated
	i = 1;											//Initialize i to an arbitrary positive value
	void * tempLocation;							//Helper pointer
	//Demonstration of the class is done through an interactive terminal interface
	//The user can enters integers to terminal to exit, allocate, or deallocate memory
	while(i > -1)	//Run until users indicates desire to exit by entering a negative numbers
	{
	head->printTree();//Show current status of tree
	cout<< "Enter a negative number to exit, 0 to allocate memory, 1 or deallocate memory: ";
	cin >> i;//User indicates desired action
	switch(i)
	{	
		case 0: {cout<< "Enter number of bytes to allocate (1 to " << MAX_MEMORY_OFFSET << "): ";
				cin>> j;	//Enter size of memory allocation
				while(j < 1 || j > MAX_MEMORY_OFFSET)	//Require user to enter valid size
				{
					cout << "Invalid value.  Please enter a valid value: ";
					cin>> j;
				}
				test = malloc(j);	//Create a block of memory to copy
				tempLocation = head->allocate(j,test);	//Attempt to copy it into the tree
				if(tempLocation)	//Check if allocation was successful
				{
					//Save the location of the memory
					k = 0;			
					while(k < 16)	//There are a max of 16 valid values; find an open index in the pointer array
					{
						if(!memLocations[k])	//save the pointer in an empty index
						{
							memLocations[k] = tempLocation;
							cout << "Allocation successful. Allocated " << j << "bytes.  Location saved in index " << k << endl; 
							k = 16;//exit loop, pointer is saved
						}
						else k++;//Iterate through pointer array
					}
					
				}
				free(test);	//No longer need the test allocation
				break;//Ask for more user input
			}
		case 1: {	//Demonstrate deallocation
				cout<<"Enter index of value to deallocate. Valid values are: ";
				bool valid = false;		//Checks to make sure there is memory to deallocate
				for(j=0; j < 16; j++)	//Find list of pointers we have saved by iterating through the pointer array
				{
					if(memLocations[j])	//Grab non-NULL pointers
					{
						valid = true;	//True if there is at least one valid pointer
						cout << j << ", ";	//Output valid index
					} 
				}
				if(valid)cout << "\b\b: ";	//formatting
				else 
				{
					cout << "Oops! The memory is empty. \n";	//Can't deallocate, need to exit
					break;//Ask for more user input
				}
				cin >> j;	//Get index to deallocate
				while(j > 15 || !memLocations[j] || j < 1)//Ask for new input until a valid index is chosen
				{
					cout << "Invalid value.  Please enter a valid value: ";
					cin >> j;
				}
				head->release(memLocations[j]);	//Release memory pointed to by memLocations[j]
				memLocations[j]=NULL;			//Indicate that location is no longer in use
				break;//Ask for more user input
			}
		case -1: break;//Exit
		default:	cout<< "Invalid value\n"; //Ask for more user input
					
		}
	}
	free(memoryBlock);	//Exiting program, free data to heap
	delete(head);		//Delete tree
	return 0;
}
/*	Method: size_t sizeToBytes()
 * 	Author: Thomas Jarvinen
 * 	Simple conversion from tree height to # of available bytes
 * input: size: tree height
 * output: size_t equivalent # of bytes
 * 
 */
size_t sizeToBytes(int size)
{
	return BASE_SIZE * pow(2,size);	//Since size is height of tree, and each layer constitutes a doubling
	//of available size, the size indicate the power of 2 over BASE_SIZE of available memory
}
/*	Method: size_t sizeToBytes()
 * 	Author: Thomas Jarvinen
 * 	Simple conversion from # of bytes to minimum node height
 * output: size: tree height
 * input:# of bytes
 * 
 */
int bytesToSize(size_t numBytes)
{
	int size = 0;
	size_t num = 1;
	while(num < numBytes)//Iterate through powers of 2 until one large enough is found
	{
		size++;
		num = num * 2;
	}
	return size-16;	//Base size is 2^16, so we must subtract 16
}
