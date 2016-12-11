#include "math.h"
#include "brightness_maps.h"
#include "legendre_polynomial.h"
#include <stdlib.h>
#include <stdio.h>

double Hotspot_b(double la, double lo,double la0, double lo0,double p_b,double spot_b,double size, int make_grid ,double theta1, double theta2, double r1, double r2, double lambda0, double phi0){

    double r_mid;
    double theta_mid;

    if(r1 == 0){
        r_mid = 0;
        theta_mid = 0;
    }
    else{
        r_mid = (r1 + r2)/2;
        theta_mid = (theta1 + theta2)/2;
    }


    if(make_grid != 0){

/*        if(r1 !=0){
            double d1  = great_circle(la0,lo0,lambda0,phi0,r1,theta1);
            double d2  = great_circle(la0,lo0,lambda0,phi0,r2,theta1);
            double d3  = great_circle(la0,lo0,lambda0,phi0,r1,theta2);
            double d4  = great_circle(la0,lo0,lambda0,phi0,r2,theta2);

            if((d1 > size*2) & (d2 > size*2) && (d3 > size*2) && (d4 > size*2)){
                printf("EASY WAY OUT\n");
                return p_b;
            }
            if((d1 < size) & (d2 < size*0.5) && (d3 < size*0.5) && (d4 < size*0.5)){
                printf("EASY WAY OUT\n");
                return spot_b;
            }
        }
*/
        int grid_len = make_grid;

        double rdiff = (r2 - r1)/grid_len;
        double thdiff = (theta2 - theta1)/grid_len;
        double R_mid, theta_mid;
//        double mid_x, mid_y;

        double total_b = 0.0;

        int inside = 0;

        for (int h = 0; h < grid_len; ++h) {
            for (int k = 0; k < grid_len; ++k) {
                theta_mid = theta1 + h*thdiff;
                R_mid = r1 + k*rdiff;
                double dist  = great_circle(la0,lo0,lambda0,phi0,R_mid,theta_mid);

                if(dist < size){
                    total_b = total_b + spot_b;
                    inside = inside +1;
                }
                else{
                    total_b = total_b + p_b;
                }

            }
        }

        total_b = total_b / pow(grid_len,2);

        return total_b;

    }

    printf("not making grid\n");

    la0 = la0*M_PI/180;
    lo0 = lo0*M_PI/180;
    double dist = acos( sin(la)*sin(la0) + cos(lo - lo0)*cos(la)*cos(la0));
    dist = dist*180/M_PI;
    if(dist < size){
        return spot_b;
    }
    else{
        return p_b;
    }

}

double great_circle(double la0,double lo0,double lambda0,double phi0,double r,double theta){

    la0 = la0*M_PI/180;
    lo0 = lo0*M_PI/180;

    double x = r*cos(theta);
    double y = r*sin(theta);
    double *coords = cart_to_ortho(1.0, x, y, lambda0, phi0);
    double la = coords[1];
    double lo = -1*coords[0];


    double dist = acos( sin(la)*sin(la0) + cos(lo - lo0)*cos(la)*cos(la0));
    free(coords);

    return dist*180/M_PI;
}

double Hotspot_T(double la, double lo,double la0, double lo0,double p_T,double spot_T,double size, int make_grid ,double theta1, double theta2, double r1, double r2, double lambda0, double phi0){

    la0 = la0*M_PI/180;
    lo0 = lo0*M_PI/180;
//    double dist = acos( sin(lo)*sin(long0) + cos(la - lambda0)*cos(lo)*cos(long0)) ;
    double dist = acos( sin(la)*sin(la0) + cos(lo - lo0)*cos(la)*cos(la0)) ;

    if(dist < size*M_PI/180){
        return spot_T;
    }

    return p_T;
}

double Uniform_b(double p_bright){
    return p_bright/M_PI;
}

double Uniform_T(double T_bright){
    return T_bright;
}

double Two_b(double la, double lo, double p_day, double p_night){
    double p_t_bright = p_night;
    if((-M_PI/2.0 <= lo) && (lo <= M_PI/2.0)){
        p_t_bright = p_day;
    }
    return p_t_bright/M_PI;
}

double Two_T(double la, double lo, double p_day, double p_night){
    double p_t_bright = p_night;
    if((-M_PI/2.0 <= lo) && (lo <= M_PI/2.0)){
        p_t_bright = p_day;
    }
    return p_t_bright;
}

double zhang_2016(double lat, double lon, double xi, double T_n, double delta_T){
    double T;
    double eta;

    // this is equation B.7 in Zhang and Showman 2016
    // Xi is the ratio of advective and radiative timescales
    // T_n is the nightside equilibrium temperature
    // delta_T is the diference between night and dayside eqt

    double phi = lat;
    double lambda = lon;

    double lambda_s = 0.;

    if(xi < 0.01){
	    if((-M_PI/2.0 <= lambda) && (lambda <= M_PI/2.0)){
		T = T_n + delta_T*cos(phi)*cos(lambda_s)*cos(lambda-lambda_s);
	    }
	    else if((-M_PI <= lambda) && (lambda <= -M_PI/2.0)){
		T = T_n;
	    }
	    else if ((M_PI/2 <= lambda) && (lambda <= M_PI)){
		T = T_n;
	    }
	    else{
		printf("lambda %f\n",lambda);
		printf("UNEXPECTED CASE IN ZHANG\n");
		return 0;
	    }
    }
    else{
	    eta = (xi/(1 + pow(xi,2)))*(exp(M_PI/(2*xi)) + exp(3*M_PI/(2*xi)))/(exp(2*M_PI/xi) - 1.0);
	    lambda_s = atan(xi);

	    if((-M_PI/2.0 <= lambda) && (lambda <= M_PI/2.0)){
		T = T_n + delta_T*cos(phi)*cos(lambda_s)*cos(lambda-lambda_s) + eta*delta_T*cos(phi)*exp(-lambda/xi);
	    }
	    else if((-M_PI <= lambda) && (lambda <= -M_PI/2.0)){
		T = T_n + eta*delta_T*cos(phi)*exp(-(M_PI+lambda)/xi);
	    }
	    else if ((M_PI/2 <= lambda) && (lambda <= M_PI)){
		T = T_n + eta*delta_T*cos(phi)*exp((M_PI-lambda)/xi);
	    }
	    else{
		printf("lambda %f\n",lambda);
		printf("UNEXPECTED CASE IN ZHANG\n");
		return 0;
	    }
    }
    return T;
}

double spherical(double lat, double lon, double *a){
    double x_vec[1];
    double fx2;
    double *fx2_vec;
    double theta = (M_PI/2.0) - (lat+a[1]);
    double phi = M_PI + (lon+a[2]);

    int orders = a[0];

    int k = 3;

    x_vec[0] = cos(theta);

    double val = 0.0;
    for (int l = 0; l < (orders); ++l) {

      for (int m = 0; m < (l+1); ++m) {
        fx2_vec = pm_polynomial_value(1,l,m,x_vec);
        fx2 = fx2_vec[l];
        free(fx2_vec);

        val = val + a[k]*cos(m*phi)*fx2;

        k = k +1;
      }
    }
    return pow(val,2);
}

double kreidberg_2016(double lat, double lon, double insol, double albedo, double redist){
    // This function calculates the temperature from equation (1) in Kreidberg & Loeb 2016 

    double sigma = 5.670367e-8;		//Stefan Boltzmann constant (W/m^2/K^4)
    double T;

    //dayside temperature
    if((-M_PI/2.0 <= lon) && (lon <= M_PI/2.0)){
        T = pow((1. - albedo)*insol*((1. - 2.*redist)*cos(lat)*cos(lon) + redist/2.)/sigma, 0.25);      
    }
    //nightside temperature
    else{
        T = pow((1. - albedo)*insol*redist/2./sigma, 0.25);    			
    }
    return T;
}
