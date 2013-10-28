#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <time.h>
#define NUMTABLES 15
using namespace std;

struct fr { 
    double w;
    int id;
  fr(int in_id, double in_w)
  {
    id=in_id;
    w=in_w;
  }
};

struct p {
  string name;
  int numFriends;
  p( string in_name, int in_numFriends)
  {
    name = in_name;
    numFriends = in_numFriends;
  }
};

class Person
{
public:
  // ids and weights of the friends
  vector<fr> friends; 
  int numPics;
  int digikamId;
  int id;
  string name;
  int table;
};

struct by_weight { 
    bool operator()(fr const &a, fr const &b) { 
        return a.w > b.w;
    }
};

struct by_weight_p { 
    bool operator()(p const &a, p const &b) { 
        return a.numFriends > b.numFriends;
    }
};

double errorFunctionBruteForce(vector<Person*>& People)
{
  double e=0;
  for (int i=0; i< People.size(); i++)
    {
      Person* person1 = People[i];
      for(int j=0; j < person1->friends.size(); j++)
	{
	  Person* person2 = People[person1->friends[j].id];
	  if (person1->table != person2->table)
	    {
	      e+=person1->friends[j].w;
	    }
	}
    }
  return e;
}

void neighbour(vector<Person*>& People, int& i, int& j)
{
  int diffTables;
  do
    {
      i = rand()%People.size();
      j = rand()%(People.size()-1);
      j = (i+j) % People.size();
    }while (People[i]->table == People[j]->table);
  swap(People[i]->table, People[j]->table);
}

void Metropolis(vector<Person*>& People, double temperature, double M)
{
  // Store the original
  vector<int> tables(People.size());
  for (int i = 0; i < People.size(); i++)
    {
      tables[i] = People[i]->table;
    }
  double besterror = errorFunctionBruteForce(People);
  bool swapped = false;
  double eold = besterror;
  double e1;
  int i,j;
  while (M > 0)
    {
      if(swapped)
	{
	  eold=e1;
	}
      else
	{
	  // unnecessary
	  //	  eold=errorFunctionBruteForce(People);
	}
      neighbour(People,i,j);
      e1 = errorFunctionBruteForce(People);
      double deltah= e1 - eold;
      double edt = exp(-deltah/temperature);
      double r = (double) rand() / RAND_MAX;
      //cout << deltah << " " << temperature << " " << e1 << " " << edt << " " << r << endl;
      
      if ( deltah < 0 || (double) rand() / RAND_MAX < exp(-deltah/temperature))
	{
	  //  cout << "swapped "<< i << " " << j << endl;
	  //  peopleMap=newpeopleMap;
	  // keep the swap
	  swapped = true;
	}
      else
	{
	  //	  	  cout << "revert"<<endl;
	  // revert
	  swap(People[i]->table, People[j]->table);
	  swapped = false;
	}
      if (e1 < besterror)
	{
	  besterror = e1;
	  for (int i = 0; i < People.size(); i++)
	    {
	      tables[i] = People[i]->table;
	    }
	}

      M--;
    }
  // Return the best
  if (errorFunctionBruteForce(People) > besterror)
    {
      for (int i = 0; i < People.size(); i++)
	{
	  People[i]->table = tables[i];
	}
    }
}

void simulatedAnnealing(vector<Person*>& People, vector<int>& peopleMap)
{
  double T0=1; // initial temperature
  double alpha = .98; // cooling rate
  double beta = 1.05; // increases the time spent in Metropolis after each temperature decrease
  double maxtime = 10000000; // max time for entire algorithm
  double M = 500; // time until the next parameter update

  double temperature=T0;
  double Time = 0;

  while (Time < maxtime)
    {
      Metropolis(People, temperature, M);
      Time += M;
      temperature *= alpha;
      M *= beta;
    }
}



bool is_number(const std::string& s)
{
  std::string::const_iterator it = s.begin();
  while (it != s.end() && std::isdigit(*it)) ++it;
  return !s.empty() && it == s.end();
}
void readMat(vector<Person*>& People)
{
  ifstream matFile("mat.txt");
  if(matFile.is_open())
    {
      while(matFile.good())
	{
	  string line;
	  getline(matFile, line);
	  istringstream iss(line);
	  vector<string> tokens;
	  copy(istream_iterator<string>(iss),
	       istream_iterator<string>(),
	       back_inserter<vector<string> >(tokens));
	  if (is_number(tokens[0]))
	    {
	      People.push_back(new Person);
	      People.back()->digikamId=atoi(tokens[0].c_str());
	      // Read in tokens until another number encountered and put that in name
	      int j=1;
	      while (j < tokens.size() && !is_number(tokens[j]))
		{
		  j++;
		}
	      string name = tokens[1];
	      for (int k=2; k < j ; k++)
		{
		  name = name+ " " + tokens[k];
		}
	      People.back()->name= name;

	      // Read in the id
	      People.back()->id = atoi(tokens.back().c_str());
	    }
	  else
	    {
	      if (tokens[0] == "AndNowForTheAssociations")
		return;
	      People.push_back(new Person);
	      People.back()->id = People.size() - 1;
	      People.back()->digikamId = -1;
	      People.back()->numPics=1;
	      int j=0;
	      while (j < tokens.size() && !is_number(tokens[j]))
		{
		  j++;
		}
	      string name = tokens[0];
	      for (int k=1; k < j ; k++)
		{
		  name = name+ " " + tokens[k];
		}
	      People.back()->name= name;
	    }
	}
    }
  matFile.close();
}

int findIdFromName(vector<Person*>& People, string name)
{
  int id =-1;
  for(int i =0; i < People.size();i++)
    {
      if (People[i]->name == name)
	{
	  id = i;
	  break;
	}
    }
  if (id ==-1)
    {
      cout << "Failed with name: " << name <<endl;
      exit(1);
    }
  return id;
}
void readExtras(vector<Person*>& People)
{
  ifstream extrasFile("extraAssociations.txt");
  if(extrasFile.is_open())
    {
      while(extrasFile.good())
	{
      string line;
      getline(extrasFile, line);
      istringstream iss(line);
      vector<string> tokens;
      string cell;
      while (getline(iss,cell,','))
	{
	  tokens.push_back(cell);
	}
      /*copy(istream_iterator<string>(iss),
	   istream_iterator<string>(),
	   back_inserter<vector<string> >(tokens));
      */
      if (tokens.size() == 0)
	{
	  return;
	}
	  string name = tokens[0];

	  // Find the id based on name
	  int id = findIdFromName(People, name);
	  // Read in the associates
	  for (int k=1; k < tokens.size(); k++)
	    {
	      int friendid = findIdFromName(People, tokens[k]);
	      People[id]->friends.push_back( fr(friendid,100) );
	    }
	  // Find the max of the friends, set it to numPics
	  // this way when we normalize, all values will be <= 1
	  // Hmmm, maybe not.  Force the extra associations to be a lot
	  // stronger than the common pictures metric
	  double n = 1;
	  for (int k=0; k < People[id]->friends.size(); k++)
	    {
	      if (People[id]->friends[k].w > n)
		{
		  n = People[id]->friends[k].w;
		}
	    }
	  //	  People[id]->numPics = n;
	  People[id]->numPics = 1;
	}
      extrasFile.close();
    }
}

void readMat2(vector<Person*>& People, vector<int>& digikamIdMap)
{
  ifstream matFile("mat.txt");
  if(matFile.is_open())
    {
      while(matFile.good())
	{
	  string line;
	  getline(matFile, line);
	  istringstream iss(line);
	  vector<string> tokens;
	  copy(istream_iterator<string>(iss),
	       istream_iterator<string>(),
	       back_inserter<vector<string> >(tokens));
	  //	  cout << tokens[0] << endl;
	  if (!is_number(tokens[0]) && tokens[0] == "AndNowForTheAssociations")
	    break;
	}
      while(matFile.good())
	{
	  string line;
	  getline(matFile, line);
	  istringstream iss(line);
	  vector<string> tokens;
	  copy(istream_iterator<string>(iss),
	       istream_iterator<string>(),
	       back_inserter<vector<string> >(tokens));
	  if (tokens.size() != 3)
	    return;

	  int id1 = atoi(tokens[0].c_str());
	  int id2 = atoi(tokens[1].c_str());
	  int w = atoi(tokens[2].c_str());
	  if (id1==207)
	    	  cout << id1 << " " << id2 << " " << w << endl;
	  if (w!=0)
	    {
	      if(id1!=id2)
		{
		  People[digikamIdMap[id1]]->friends.push_back(fr(digikamIdMap[id2],w));
		}
	      else
		{
		  People[digikamIdMap[id1]]->numPics = w;
		}
	    }
	}
    }
  matFile.close();
}

void displaySeatings(vector<Person*>& People, vector<int>& peopleMap)
{
  vector< vector< int > > s(NUMTABLES);
  for (int i = 0; i < People.size(); i++)
    {
      int id = People[i]->id;
      int table = People[i]->table;
      s[table].push_back(id);
    }
  /*
    for (int i =0; i < s.size(); i++)
    {
      cout << "Table " << i << ":"<<endl;
      for (int j =0; j < s[i].size(); j++)
	{
	  cout << People[s[i][j]]->name << endl;
	  //	  cout << People[peopleMap[s[i][j]]]->name << endl;
	}
    }

  */
  for (int j = -1; j < 8; j++)
    {
      for (int i=0; i < 8; i++)
	{
	  if (j==-1)
	    cout << "Table " << i << "             ";
	  else
	    {
	      int r = 20;
	      if ( s[i].size() > j)
		{
		  cout << People[s[i][j]]->name;
		  r = 20 - People[s[i][j]]->name.size();
		}
	    for (int k = 0; k < r; k++)
	      {
		cout << " " ;
	      }
	    }
	}
      cout << endl;
    }

  cout << endl;
  for (int j = -1; j < 8; j++)
    {
      for (int i=8; i < NUMTABLES; i++)
	{
	  if (j==-1)
	    {
	    if( i < 10)
	    {
	    cout << "Table " << i << "             ";
	    }
	  else
	    {
	    cout << "Table " << i << "            ";

	    }
	    }
	  else
	    {
	      int r =20;
	      if ( s[i].size() > j)
		{
		  cout << People[s[i][j]]->name;
		  r = 20 - People[s[i][j]]->name.size();
		}
	      for (int k = 0; k < r; k++)
		{
		  cout << " " ;
		}
	    }
	}
      cout << endl;
    }
}

void createMetisFile(vector<Person*>& People)
{
  // Since Metis only deals with undirected graphs, we can transform our directed graph to an
  // undirected graph by replacing making the weights the average of the two weights.
  // wab=wba = (wab+wba)/2
  // Sums are perserved.
  vector< vector<double> > directedMatrix(People.size(), vector<double> (People.size()));
  vector< vector<double> > undirectedMatrix(People.size(), vector<double> (People.size()));

  for (int i=0; i< People.size(); i++)
    {
      Person* person1 = People[i];
      directedMatrix[i][i] = person1->numPics;
      for(int j=0; j < person1->friends.size(); j++)
	{
	  Person* person2 = People[person1->friends[j].id];
	  directedMatrix[i][person1->friends[j].id]=person1->friends[j].w;
	}
    }

  for (int i=0; i< People.size(); i++)
    {
      for (int j=0; j< People.size(); j++)
	{
	  undirectedMatrix[i][j] = (directedMatrix[i][j] + directedMatrix[j][i])/2;
	}
    }

  int numEdges = 0;
  for (int i=0; i< People.size(); i++)
    {
      for (int j=i+1; j< People.size(); j++)
	{
	  if (undirectedMatrix[i][j] != 0)
	    {
	      numEdges++;
	    }
	}
    }

  ofstream metisFile;
  metisFile.open("metisGraph.txt");
  metisFile << People.size() << " " << numEdges << " 001" << endl;
  for (int i=0; i< People.size(); i++)
    {
      bool firstEntry = true;
      for (int j=0; j< People.size(); j++)
	{
	  if (i==j)
	    {
	      continue;
	    }
	  if (undirectedMatrix[i][j] != 0)
	    {
	      if (!firstEntry)
		{
		  metisFile << " ";
		}
	      firstEntry=false;
	      metisFile << j+1 << " " << (int) (1000*fabs(undirectedMatrix[i][j]));
	    }
	}
      metisFile << endl;
    }
  metisFile.close();
}

void OptimumSwaps(vector<Person*>& People)
{
  bool canImprove=false;
  do{
    double besti=-1;
    double bestj=-1;
    double bestError = errorFunctionBruteForce(People);
    canImprove=false;
    for (int i=0; i < People.size()-1; i++)
      {
	for (int j=i+1; j < People.size(); j++)
	  {
	    if (People[i]->table == People[j]->table)
	      {
		continue;
	      }

	    //	    vector<int> newPeopleMap = peopleMap;
	    //	    swap(newPeopleMap[i], newPeopleMap[j]);
	    swap(People[i]->table, People[j]->table);
	    double e = errorFunctionBruteForce(People);
	    swap(People[i]->table, People[j]->table);

	    // If you really hate your guests, you can create the
	    // worst possible seating arrangement by changing < to >.
	    if (e < bestError)
	      {
		bestError = e;
		besti=i;
		bestj=j;
		canImprove=true;
	      }
	  }
      }
    if (canImprove)
      {
	cout << bestError << " " << besti << " " << 
	  People[besti]->name << " " << bestj << 
	  " " << People[bestj]->name  << endl;
	
	//	swap(peopleMap[besti], peopleMap[bestj]);
	swap(People[besti]->table, People[bestj]->table);
      }
  }while (canImprove);
}

void readMetisFileOutput(vector<Person*>& People)
{
  ifstream matisFile("metisGraph.txt.part.NUMTABLES");
  if(matisFile.is_open())
    {
      int i=0;
      while(matisFile.good())
	{
	  string line;
	  getline(matisFile, line);
	  if (line == "")
	    {
	      break;
	    }
	  People[i]->table = atoi(line.c_str());
	  i++;
	}
    }
}


int main()
{
  srand (time(NULL));
  // Read in the first 120 lines of mat.txt
  // only pay attention to lines that start with a number
  vector<Person*> People;

  readMat(People);
  readExtras(People);
  vector<int> digikamIdMap(710, -1); // The max digikamId is 709 (Anna Balodis)  cat mat.txt | awk '{print $1}'| sort -n
  for (int i =0 ; i < People.size(); i++)
    {
      if (People[i]->digikamId != -1)
	digikamIdMap[People[i]->digikamId] = People[i]->id;

    }

  readMat2(People, digikamIdMap);
  for (int i =0 ; i < People.size(); i++)
    {
      sort(People[i]->friends.begin(), People[i]->friends.end(), by_weight());
      cout << "**** " << " " << People[i]->name << " id " << People[i]->id << " digikamId " << People[i]->digikamId  <<" number of friends " << People[i]->friends.size() 
	   << " number of pics total " << People[i]->numPics << " has friends:";
      for (int j =0; j < People[i]->friends.size(); j++)
	{
	  cout << " " << People[People[i]->friends[j].id]->name << " count " << People[i]->friends[j].w;
	}
      cout << endl;
    }
  
  cout << endl;
  // Sort friends by friend size
  cout << "Friends shown by number of people they will know at the wedding (not counting extra associations)"<< endl;
  vector<p> vp;
  for (int i =0; i < People.size(); i++)
    {
      vp.push_back( p(People[i]->name, People[i]->friends.size()));
    }
  sort(vp.begin(), vp.end(), by_weight_p());

  for (int i =0; i < vp.size(); i++)
    {
      cout << vp[i].name << " " << vp[i].numFriends<< endl;
    }
  cout << endl;



    // Way of indexing the people so we can find them by id in O(1)
    // Person with id=i is People[peopleMap[i]]
    vector<int> peopleMap(People.size());
    for (int i =0; i < peopleMap.size(); i++)
      {
	peopleMap[i]=i;
	People[i]->table = i % NUMTABLES;
	cout << i << " " << People[i]->name << " ";
	// Normalize the weights
	for (int j =0; j < People[i]->friends.size(); j++)
	  {
	    People[i]->friends[j].w /= People[i]->numPics;
	    cout << People[i]->friends[j].w << " ";
	  }
	cout << endl;
      }

    double eorig = errorFunctionBruteForce(People);
    cout << "attempting annealing " << endl;
    simulatedAnnealing(People, peopleMap);
    double esimmulatedAnnealing = errorFunctionBruteForce(People);
    displaySeatings(People, peopleMap);

    cout << "attempting brute force " << endl;
    OptimumSwaps(People);
    cout << "Optimum seating (Chris's algorithm)" << endl;
    double eOptimumSwaps = errorFunctionBruteForce(People);
    displaySeatings(People, peopleMap);
    

    createMetisFile(People);
    system("metis-5.0.3/build/Linux-x86_64/programs/gpmetis metisGraph.txt NUMTABLES");
    readMetisFileOutput(People);
    double eMetis = errorFunctionBruteForce(People);
    displaySeatings(People, peopleMap);

    cout << "Original seating plan error: " << eorig << endl;
    cout << "After simulated annealing seating plan error: " << esimmulatedAnnealing << endl;
    cout << "Optimum swaps error: " << eOptimumSwaps << endl;
    cout << "Metis seating plan error: " << eMetis << endl;
    /*    OptimumSwaps(People, peopleMap);
    displaySeatings(People, peopleMap);

*/
    /*    for (int i =0; i < peopleMap.size(); i++)
      {
	cout << i << " " << peopleMap[i] << " " << People[peopleMap[i]]->id << endl;

	}*/
}

