#include "Solver.h"
#include "Grid.h"
#include "Grid_int.h"
#include "Stencil.h"
#include <fstream>
#include<omp.h>


Solver::Solver(int &l_level)
{
    level= l_level;
    //Vcycle = n_Vcycle;

    ngp_= pow(2,level)+1;
}

Solver::Solver(int &l_level, std::vector<double> &u)
{

level= l_level;
//Vcycle = n_Vcycle;

ngp_= pow(2,level)+1; 
u_initial = u;

for(int i= level;i>0;--i)
	{
	Grid *g =  new Grid(i);
	lev_Vec.push_back(g);
	}
lev_Vec[0]->u_app = u_initial;
}

//------------------------------getu_app----------------------------------------------------------//

std::vector<double> Solver::get_u_app(int &l_level)
{
 int x=level- l_level;
 std::vector<double> z= lev_Vec[x]->u_app;
 return z;
}

//------------------------------ get_res -------------------------------------//

std::vector<double> Solver::get_res(int &l_level){
    int x=level- l_level;
   
    std::vector<double> z= lev_Vec[x]->res;
    return z;
}

///*********************************MAPPING FUNCTION**********************************************//

int map(int i,int j,int ngl)
{
return  (i*ngl)+j;
}


///*********************************PRINT FUNCTIONS**********************************************//

void Solver::display_u()
{
std::cout<< "level "<< level << std::endl; 
//ngp_= pow(2,level)+1; 
for(size_t i=0; i<ngp_; ++i)
	{
			{	
			for(size_t j=0; j<ngp_;++j)
			std::cout<< u_initial[i*ngp_+j] << "\t";
			}
	std::cout<<std::endl;
	}
}


void Solver::display_u_app(int l_level)
{
std::cout<< '\n' <<"U approximation at level " << l_level << std::endl;
int x = level-l_level;

double ngl_=lev_Vec[x]->get_ngpValue();

//double ngl_= pow(2,l_level)+1;

for(size_t i=0; i<ngl_; ++i)
	{
		{	
		for(size_t j=0; j<ngl_;++j)
		std::cout<< lev_Vec[x]->u_app[i*ngl_+j] << "\t";
		}
	std::cout<<std::endl;
	}
}




void Solver::display_frc(int l_level)
{
std::cout<< '\n'<< "Force vector at level " << l_level << std::endl;

int x = level-l_level;
double ngl_= lev_Vec[x]->Grid::get_ngpValue();


for(size_t i=0; i<ngl_; ++i)
	{
		{	
		for(size_t j=0; j<ngl_;++j)
		std::cout<< lev_Vec[x]->frc[i*ngl_+j] << "\t";
		}
	std::cout<<std::endl;
	}
}


void Solver::display_res(int l_level)
{
std::cout<< '\n' << "Residual vector at level " << l_level << std::endl;

int x = level-l_level;
double ngl_= lev_Vec[x]->Grid::get_ngpValue();


for(size_t i=0; i<ngl_; ++i)
	{
		{	
		for(size_t j=0; j<ngl_;++j)
		std::cout<< lev_Vec[x]->res[i*ngl_+j] << "\t";
		}
	std::cout<<std::endl;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///********************************* RED-BLACK G.S. *********************************************///
////////////////////////////////////////////////////////////////////////////////////////////////////

void Solver::RBGS(int &l_level)
{
int x=level-l_level;
double ngl_= lev_Vec[x]->Grid::get_ngpValue();
double h2_= 4.0/((ngl_-1)*(ngl_-1));
int midval = 0.5*(ngl_-1);
///---------------------------------- RED UPDATE -------------------------------------------------//
//int i,j;

int k= ngl_-1;

omp_set_num_threads(4);
#pragma omp parallel
{
#pragma omp for

    for(int i=1;i<k;++i)
    {
        if(i & 1)
        {

            for(int j=1;j<k;j+=2)
            {
                if(i==midval && j>=midval)
                        {
                    lev_Vec[x]->u_app[map(i,j,ngl_)]=0.0;
                        }
                else
                       {
                       lev_Vec[x]->u_app[map(i,j,ngl_)] = 0.25*(lev_Vec[x]->u_app[map(i,j-1,ngl_)]	+
								lev_Vec[x]->u_app[map(i,j+1,ngl_)]	+
								lev_Vec[x]->u_app[map(i-1,j,ngl_)]	+
								lev_Vec[x]->u_app[map(i+1,j,ngl_)]	+
                                                                (h2_ * lev_Vec[x]->frc[map(i,j,ngl_)]));
                        }
            }
        }
        else
        {
            for(int j=2;j<k;j+=2)
            {
                if(i==midval && j>=midval)
                    {
                    lev_Vec[x]->u_app[map(i,j,ngl_)]=0.0;
                    }
                else
                    {
                        lev_Vec[x]->u_app[map(i,j,ngl_)] = 0.25*(lev_Vec[x]->u_app[map(i,j-1,ngl_)]	+
								lev_Vec[x]->u_app[map(i,j+1,ngl_)]	+
								lev_Vec[x]->u_app[map(i-1,j,ngl_)]	+
								lev_Vec[x]->u_app[map(i+1,j,ngl_)]	+
                                                                (h2_ * lev_Vec[x]->frc[map(i,j,ngl_)]));
                    }
            }
        }

	}    
}

///---------------------------------- BLACK UPDATE -----------------------------------------------//


omp_set_num_threads(4);
#pragma omp parallel
{
#pragma omp for
    for(int i=1;i<k;++i)
    {
        if(i & 1)
        {
            for(int j=2;j<k;j+=2)
            {
                if(i==midval && j>=midval)
                { lev_Vec[x]->u_app[map(i,j,ngl_)]=0.0;}
                else
                {

                            lev_Vec[x]->u_app[map(i,j,ngl_)] = 0.25*(lev_Vec[x]->u_app[map(i,j-1,ngl_)]	+
                                                                    lev_Vec[x]->u_app[map(i,j+1,ngl_)]	+
                                                                    lev_Vec[x]->u_app[map(i-1,j,ngl_)]	+
                                                                    lev_Vec[x]->u_app[map(i+1,j,ngl_)]	+
                                                                    (h2_*lev_Vec[x]->frc[map(i,j,ngl_)]));
                }
            }
        }
        else
        {
            for(int j=1;j<k;j+=2)
            {
                if(i==midval && j>=midval)
                { lev_Vec[x]->u_app[map(i,j,ngl_)]=0.0;}
                else
                {
                    lev_Vec[x]->u_app[map(i,j,ngl_)] = 0.25*(lev_Vec[x]->u_app[map(i,j-1,ngl_)]	+
								lev_Vec[x]->u_app[map(i,j+1,ngl_)]	+
								lev_Vec[x]->u_app[map(i-1,j,ngl_)]	+
								lev_Vec[x]->u_app[map(i+1,j,ngl_)]	+
                                                                (h2_*lev_Vec[x]->frc[map(i,j,ngl_)]));
                }
            }
        }


    }
}
}
///***************************** SMOOTHING FUNCTIONS *********************************************//

void Solver::pre_smoothing(int &l_level)
{
    for(int i=0;i<2;++i)
        this->RBGS(l_level);
}

void Solver::post_smoothing(int &l_level)
{
	this->RBGS(l_level);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///*********************************** RESIDUAL **************************************************//
////////////////////////////////////////////////////////////////////////////////////////////////////

void Solver::residual(int &l_level)
{

int x=level-l_level;
double ngl_= lev_Vec[x]->Grid::get_ngpValue();
double h2_ = ((ngl_-1)*(ngl_-1))/4.0; // h2_ =(h^2)
//std::cout<< "h sqr "<< h2_ <<std::endl ;
//double norm=0;
//double temp=0;
int midval = 0.5*(ngl_-1);
int k =ngl_-1;
omp_set_num_threads(4);
#pragma omp parallel
{
#pragma omp for

for(int i=1;i<k;++i)
	{
    for(int j=1;j<k;++j)
		{
                if(i==midval && j>=midval){lev_Vec[x]->res[map(i,j,ngl_)]=0.0;}
                else{

		//std::cout<< "i "<< i <<" j " << j << " to map " << map(i,j,ngl_) << std::endl ;
		lev_Vec[x]->res[map(i,j,ngl_)]=	lev_Vec[x]->frc[map(i,j,ngl_)]+	(h2_*(lev_Vec[x]->u_app[map(i,j-1,ngl_)]		+
										lev_Vec[x]->u_app[map(i,j+1,ngl_)]			+
										lev_Vec[x]->u_app[map(i-1,j,ngl_)]			+
										lev_Vec[x]->u_app[map(i+1,j,ngl_)]			-
										(4*lev_Vec[x]->u_app[map(i,j,ngl_)])));
                    }
		}
	
	}
 }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///*********************************** RESTRICTION ***********************************************//
////////////////////////////////////////////////////////////////////////////////////////////////////


void Solver::restriction(int &l_level)
{
int x=level-l_level;
double ngl_= lev_Vec[x]->get_ngpValue();
double ngl_1dwn = lev_Vec[x+1]->get_ngpValue();
Stencil_rest r;


for(int i=0;i<ngl_;++i)
{
	if(i%2==0)
	{
	for(int j=0;j<ngl_;++j)
	{
		if(j%2==0)
		{	
			lev_Vec[x+1]->u_app[map(i/2,j/2,ngl_1dwn)]=0;	
		}
	}
}
}



for(int i=2;i<ngl_-2;i+=2)
{
	for(int j=2;j<ngl_-2;j+=2)
	{
       // if(i==ngl_*0.5 && j>=ngl_*0.5){;}
        //else{

	lev_Vec[x+1]->frc[map(i/2,j/2,ngl_1dwn)]=    	r.W*lev_Vec[x]->res[map(i,j-1,ngl_)] 		+ 
							r.C*lev_Vec[x]->res[map(i,j,ngl_)]   		+ 
							r.E*lev_Vec[x]->res[map(i,j+1,ngl_)]  		+ 
							r.SW*lev_Vec[x]->res[map(i-1,j-1,ngl_)] 	+ 
							r.S*lev_Vec[x]->res[map(i-1,j,ngl_)] 		+ 
							r.SE*lev_Vec[x]->res[map(i-1,j+1,ngl_)] 	+ 
							r.N*lev_Vec[x]->res[map(i+1,j,ngl_)] 		+ 
							r.NW*lev_Vec[x]->res[map(i+1,j-1,ngl_)] 	+ 
							r.NE*lev_Vec[x]->res[map(i+1,j+1,ngl_)];
        //}
        }
}

}


////////////////////////////////////////////////////////////////////////////////////////////////////
///*********************************** PROLONGATION **********************************************//
////////////////////////////////////////////////////////////////////////////////////////////////////

void Solver::prolongation(int &l_level)
{
int x=level-l_level; 
double ngl_= lev_Vec[x]->get_ngpValue();   //coarse grid
double ngl_up = lev_Vec[x-1]->get_ngpValue();  //fine grid
size_t midval = 0.5*(ngl_ -1);
Stencil_prol p;

for(size_t i=1;i<ngl_-1;i++)
	{
	for(size_t j=1;j<ngl_-1;j++)
        {

        if(i==midval && j>=midval)
	{
	lev_Vec[x-1]->u_app[map(2*i,2*j,ngl_up)]=0.0;
	}

        else{

        lev_Vec[x-1]->u_app[map(2*i-1,2*j-1,ngl_up)]	+= 	p.SW*lev_Vec[x]->u_app[map(i,j,ngl_)];
		lev_Vec[x-1]->u_app[map(2*i-1,2*j,ngl_up)]	+= 	p.S*lev_Vec[x]->u_app[map(i,j,ngl_)];
		lev_Vec[x-1]->u_app[map(2*i-1,2*j+1,ngl_up)]  	+= 	p.SE*lev_Vec[x]->u_app[map(i,j,ngl_)];
		lev_Vec[x-1]->u_app[map(2*i,2*j-1,ngl_up)] 	+= 	p.W*lev_Vec[x]->u_app[map(i,j,ngl_)];
		lev_Vec[x-1]->u_app[map(2*i,2*j,ngl_up)]         	+= 	p.C*lev_Vec[x]->u_app[map(i,j,ngl_)];
		lev_Vec[x-1]->u_app[map(2*i,2*j+1,ngl_up)] 	+= 	p.E*lev_Vec[x]->u_app[map(i,j,ngl_)];
		lev_Vec[x-1]->u_app[map(2*i+1,2*j-1,ngl_up)] 	+=    	p.NW*lev_Vec[x]->u_app[map(i,j,ngl_)];
		lev_Vec[x-1]->u_app[map(2*i+1,2*j,ngl_up)] 	+= 	p.N*lev_Vec[x]->u_app[map(i,j,ngl_)];
		lev_Vec[x-1]->u_app[map(2*i+1,2*j+1,ngl_up)] 	+= 	p.NE*lev_Vec[x]->u_app[map(i,j,ngl_)];

        }
        }
	}

}

///----------------------------------Store-------------------------------------------------------///

void Solver::store(double ngp,std::vector<double>u,std::vector<double>u_inti)//,std::vector<double>error)
{
double hgl_ = 2.0 / (ngp-1);

std::ofstream mg,inti,err;

mg.open("solution.dat");

for(double y=-1,i=0; y<=1; y+=hgl_,i++)
{
    for(double x=-1,j=0; x<=1;x+=hgl_,++j)
        mg<< x<<"\t"<< y<<"\t" <<u[i*ngp+j] << "\n";
}
mg.close();

//--------- store inti.dat ------------  

inti.open("init.dat");
for(double y=-1,i=0; y<=1; y+=hgl_,i++){
    for(double x=-1,j=0; x<=1;x+=hgl_,++j)
          inti << x << "\t" << y <<"\t" <<u_inti[i*ngp+j]<<"\n";
	}
inti.close();

/*
err.open("error.dat");
for(double y=-1,i=0; y<=1; y+=hgl_,i++){
    for(double x=-1,j=0; x<=1;x+=hgl_,++j)
          err << x << "\t" << y <<"\t" <<error[i*ngp_+j]<<"\n";
        }
err.close();
*/
}

//------------------------------------  Residual Norm --------------------//
/*
double Solver::normResidual(std::vector<double> f_res,double previous_res){
static int count=1;
double res=0,norm_res;


for(size_t i=0;i<f_res.size();i++){

        res+=(f_res[i] * f_res[i]);

    }

    //div_res = res / (f_res.size() * f_res.size());

    norm_res = sqrt(res / (f_res.size()));
   std::cout<<"\n Norm of residual after V-cycle ....   "<< norm_res<<std::endl;
	
	if(count>1)
		{
			double q = norm_res / previous_res;
            std::cout<<" Convergence q rate after V-cycle ....   "<< q <<std::endl;
		}


   
	count++;

return norm_res;
}
*/
//--------------------------------------  Error task-5 ----------------------------------------------//
/*
long double error(std::vector<double>u_h,std::vector<double>u_exact)
{

long double norm=0.0, norm1=0.0;

    for(size_t k=0;k<u_h.size();k++)
        {

        //std::cout<<"k is ... "<<k<<"\t u appri.. "<<u_h[k]<<"\t u exact.. "<<u_exact[k]<<"\t differ.... "<<(u_h[k]-u_exact[k])<<std::endl;
        //norm+=((u_h[k]-u_exact[k])*(u_h[k]-u_exact[k]));

        norm1 =(u_exact[k]-u_h[k]);
	norm += norm1 * norm1;

        }
norm = (norm/u_h.size());
//std::cout<<"u h size is ......."<< u_h.size()<<std::endl;
//std::cout<<"Error Norm is ......."<< sqrt(norm)<<std::endl;
return (sqrt(norm));

}
*/
//-----------------------------------------------------------------------------------------------------------------------------------------------//


//////////////////////////////Simulation///////////////////////////////////////////////////

std::vector<double> Solver::Simulation()
{

int l_Level = this-> level;
//int n_Vcycle = this-> Vcycle;

//double r[2];   // store residual norm value 
/// Initialise grid.
Grid_int v(l_Level);

/// Apply Boundary conditions.
v.boundary_con();

std::vector<double> u_exact=v.U_exact();


std::vector<double> u = v.get_Xvalue();
//v.display_grid_int();
std::vector<double> u_inti = v.get_Xvalue();

//long double er = 1.0;
 
int i=1;

//for(int i=1; i<=n_Vcycle; ++i)
//long double E =9.18e-5;

while(i < 14)
  {
  //std::cout<< "\n Current V-cycle " << i <<std::endl;
 
    Solver S(l_Level,u);

          for (int j =l_Level; j>0; --j) // Pre Smoothning -> U Print -> Residual -> Residual print -> Restriction -> Force Print
        {
        S.pre_smoothing(j); //2*S.RBGS(j);

 	 S.residual(j);
  
        if(j!=1)
        {
        S.restriction(j);
       }
    }
	int m=1;
        S.post_smoothing(m);
   
  
    for (int k =1; k<l_Level; ++k) // Prolongation -> U Print +1 Level -> Residual -> Residual print -> Restriction -> Force Print
    {

        S.prolongation(k);
 
        if(k!=1)
        {
	    int k1=k+1; //edit
            S.post_smoothing(k1);
  
        }
    

}

 	u=S.get_u_app(l_Level);
	

	//S.residual(l_Level);
	
	std::vector<double> f_res=S.get_res(l_Level);
	
	
//	r[i]=S.normResidual(f_res,r[i-1]);

	//er = error(u,u_exact);

	//std::cout<<"ER: "<<er<<std::endl;
	i++;

/*
std::cout<<"\n U exact solution ...."<<std::endl;
	
for(size_t i=0; i<ngp_; ++i)
{
    {
    for(size_t j=0; j<ngp_;++j)
    std::cout<< u_exact[i*ngp_+j] << "\t";
    }
std::cout<<std::endl;
}
*/
/*
std::cout<<"\n U Appro. solution ...."<<std::endl;


for(size_t i=0; i<ngp_; ++i)
{
    {
    for(size_t j=0; j<ngp_;++j)
    std::cout<< u[i*ngp_+j] << "\t";
    }
std::cout<<std::endl;
}
*/

}

//std::vector<double> err;
/*
for(size_t p=0;p<u.size();p++)
{
     err.push_back(u_exact[p]-u[p]);
}
*/
//error(u,u_exact);



//store(ngp_,u,u_exact);//,err);
return u;
}

/*
void Solver::write()
{
    std::vector<double> u=S.get_u_app(l_Level);
   std::vector<double> u_exact=v.U_exact();
  store(ngp_,u,u_exact);//,err);
}*/

Solver::~Solver()
{
}

