#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <iostream>
#include "../RMSDTools.h"
#include "../RMSDomp.h"


using namespace std;

void load_vector(vector<double> & , const char * );
void print_vector(const char*,double*, int);
bool expectedVectorEqualsCalculatedWithinPrecision(const double * const , const double * const , int , double );
void compareVectors(const char*, const double * const , const double * const , int , double );

void test_initialize(){
	cout <<"\nTEST test_initialize"<<endl;
	double expected_initialized[] = {3,3,3,3,3,3,3,3,3,3};
	double initialiazed[10];
	RMSDTools::initializeTo(initialiazed, 3, 10);
	compareVectors("\tTesting initialization: ", expected_initialized, initialiazed, 10, 1e-10);
}

void test_copy_array(){
	cout <<"\nTEST test_copy_array"<<endl;
	double expected_array[] = {0,1,2,3,4,5,1234,7,8,9};
	double uninitialized_array[10];
	RMSDTools::copyArrays(uninitialized_array, expected_array, 10);
	compareVectors("\tTesting array copy: ", expected_array, uninitialized_array, 10, 1e-10);
}

void test_coordinates_mean(){
	cout <<"\nTEST test_coordinates_mean"<<endl;
	double mean_coordinates[9];
	double coordinates[] = { 1,1,1,  2,2,2,  3,3,3,  // 1st conformation
							  4,4,4,  5,5,5,  6,6,6,  // 2nd conformation
							  7,7,7,  8,8,8,  9,9,9 };// 3rd conformation
	double expected_mean_coordinates[] = {4, 4, 4, 5, 5, 5, 6, 6, 6};
	int number_of_atoms = 3;
	int number_of_conformations = 3;
	RMSDTools::calculateMeanCoordinates(mean_coordinates, coordinates,
				number_of_conformations, number_of_atoms);

	compareVectors("\tTesting coordinates mean: ", expected_mean_coordinates, mean_coordinates, 9, 1e-10);
}

void test_superposition_with_coordinates_change(){
	cout <<"\nTEST test_superposition_with_coordinates_change"<<endl;
	double coordinates[] = { 1,1,0,  2,2,0,  3,3,0,  // 1st conformation
							  6,6,0,  5,5,0,  4,4,0,  // 2nd conformation (upside down)
							  7,7,0,  8,8,0,  9,9,0 , // 3rd conformation
							  5,4,0,  5,5,0,  4,4,0}; // 4th conformation (upside down + one changed point
													  // with dist d((6,6),(5,4))
	double reference_coordinates[] = {4,4,0,  5,5,0,  6,6,0};
	double rmsds[3];
	int number_of_atoms = 3;
	int number_of_conformations = 4;

	double expected_reference[] = {4,4,0,  5,5,0,  6,6,0};
	double expected_rmsds[] = {0, 0, 0, 0.913766};
	double expected_coordinates[] = {4,4,0,  5,5,0,  6,6,0,
									  4,4,0,  5,5,0,  6,6,0,
			                          4,4,0,  5,5,0,  6,6,0,
			                          4.5286,5,0,  5.2357,4.29289,0,  5.2357,5.70711,0};

	RMSDomp calculator(number_of_conformations,number_of_atoms, coordinates);
	calculator.superpositionChangingCoordinates(reference_coordinates,rmsds);

	//print_vector("references:",reference_coordinates, number_of_atoms*3);

	compareVectors("\tTesting RMSD: ", expected_rmsds, rmsds, number_of_conformations, 1e-4); // Only the fourth decimal, as it was obtained with cout without more precission:P
	compareVectors("\tTesting coordinates: ", expected_coordinates, coordinates,
				number_of_atoms*3*number_of_conformations, 1e-4);
	compareVectors("\tTesting reference: ", expected_reference, reference_coordinates, number_of_atoms*3, 1e-10);
}

void test_vector_len(vector<double>& v, unsigned int expected_len, const char* name){
	if(v.size()!= expected_len){
		cout<<name<<" size is "<<v.size()<<" instead of "<<expected_len<<endl;
	}
}

void test_superposition_with_different_fit_and_calc_coordsets(){
	cout <<"\nTEST test_superposition_with_different_fit_and_calc_coordsets"<<endl;
	int number_of_coordsets = 5;
	int number_of_atoms = 3239;
	int number_of_CAs = 224;
	// rmsds 1 vs others [ 0.        ,  1.86745533,  2.07960877,  3.60601759,  2.18942902]
	vector<double> not_aligned_CA;
	vector<double> not_aligned_coordinates;
	vector<double> aligned_coordinates;
	double rmsds[5];
	RMSDTools::initializeTo(rmsds, 0., 5);

	load_vector(not_aligned_CA, "data/ligand_mini_CAs");
	load_vector(not_aligned_coordinates, "data/ligand_mini_all");
	load_vector(aligned_coordinates, "data/ligand_mini_all_aligned");

	test_vector_len(not_aligned_CA, number_of_CAs*number_of_coordsets*3, "not aligned CAs");
	test_vector_len(not_aligned_coordinates, number_of_atoms*number_of_coordsets*3, "all not aligned atoms");
	test_vector_len(aligned_coordinates, number_of_atoms*number_of_coordsets*3, "all aligned atoms");

	// RMSD of all atoms using CA for superposition
	RMSDomp calculator(number_of_coordsets, number_of_CAs, &(not_aligned_CA[0]));
	calculator.setCalculationCoordinates(number_of_atoms, &(not_aligned_coordinates[0]));
	calculator.oneVsFollowing(0, rmsds);
	//print_vector("rmsd:",rmsds, 5);
	double expected_rmsds []= {1.864003731005552, 2.076760850428891, 3.596135117728627, 2.182685209336899, 0};
	compareVectors("\tTesting RMSD 1: ", expected_rmsds, rmsds, number_of_coordsets, 1e-10); // Only the fourth decimal, as it was obtained with cout without more precission:P

	// RMSD of CA using CA for superposition (default behaviour)
	RMSDomp calculator2(number_of_coordsets, number_of_CAs, &(not_aligned_CA[0]));
	calculator2.oneVsFollowing(0, rmsds);
//	print_vector("rmsd:",rmsds, 5);
	double expected_rmsds_2 []= {0.767947519172927, 0.8838644164683896, 0.4177715823462121, 0.3383320758562839, 0};
	compareVectors("\tTesting RMSD 2: ", expected_rmsds_2, rmsds, number_of_coordsets, 1e-10);

	// RMSD  of CA using CA for superposition (using the same selection and RMSD subsets)
	RMSDomp calculator3(number_of_coordsets, number_of_CAs, &(not_aligned_CA[0]));
	calculator.setCalculationCoordinates(number_of_CAs, &(not_aligned_CA[0]));
	calculator.oneVsFollowing(0, rmsds);
//	print_vector("rmsd:",rmsds, 5);
	compareVectors("\tTesting RMSD 3: ", expected_rmsds_2, rmsds, number_of_coordsets, 1e-10);

}

/*void test_translations(){

}*/

void test_iterposition(){

}

int main(int argc, char **argv){

	test_initialize();
	test_copy_array();
	test_coordinates_mean();
	test_superposition_with_coordinates_change();
	test_superposition_with_different_fit_and_calc_coordsets();

	return 0;
}



void compareVectors(const char* message, const double * const expectedVector, const double * const calculatedVector, int dimension, double precision){
	cout<<message;
	bool comparison = expectedVectorEqualsCalculatedWithinPrecision(expectedVector,calculatedVector,dimension,precision);

	if(comparison == true){
		cout<<"OK"<<endl;
	}
	else{
		cout<<"KO"<<endl;
	}
}

bool expectedVectorEqualsCalculatedWithinPrecision(const double * const expectedVector, const double * const calculatedVector, int dimension, double precision){
	bool equal = true;

    for(int i=0;i<dimension;++i)
    {
        if( fabs(expectedVector[i]-calculatedVector[i]) >= precision )
        {
            equal = false;

            cout<<setprecision(16)<<"Problem: expectedVector["<<i<<"]="<<expectedVector[i]<<
            		" calculatedVector["<<i<<"]="<<calculatedVector[i]<<endl;
            cout<<" (dif = "<<fabs(expectedVector[i]-calculatedVector[i])<<")"<<endl;
            break;
        }
    }
    return equal;
}

void print_vector(const char*message, double*  rmsd_vec, int len){
	cout<<message<<" ";
	for(int i =0; i< len; ++i){
		cout<<rmsd_vec[i]<<" ";
	}
	cout<<flush<<endl;
}


inline double toDouble(const std::string & s){
	std::istringstream i(s);
	double x;
	i >> x;
	return x;
}

void load_vector(vector<double> & vector, const char * dataPath){
	string line;

	ifstream myfile (dataPath);

	if (myfile.is_open())	{
		while(getline(myfile, line)){
				vector.push_back(toDouble(line));
		}

		myfile.close();
	}
	else{
		cout << "Unable to open file: "<< string(dataPath) << endl;
	}
}