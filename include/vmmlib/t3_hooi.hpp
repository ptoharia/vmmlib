/* 
 * VMMLib - Tensor Classes
 *  
 * @author Susanne Suter
 *
 * The tucker3 tensor class is consists of the same components (core tensor, basis matrices u1-u3) as the tucker3 model described in:
 * - Tucker,  “Some mathematical notes on three-mode factor analysis”, Psychometrika, vol. 31, no. 3, pp. 279–311., 1966 Sep.
 * - De Lathauwer L., De Moor B., Vandewalle J., ``A multilinear singular value decomposition'', 
 * SIAM J. Matrix Anal. Appl., vol. 21, no. 4, Apr. 2000, pp. 1253-1278.
 * - De Lathauwer L., De Moor B., Vandewalle J., ``On the Best rank-1 and Rank-$(R_1, R_2, ..., R_N)$ Approximation and Applications of Higher-Order Tensors'', 
 * SIAM J. Matrix Anal. Appl., vol. 21, no. 4, Apr. 2000, pp. 1324-1342.
 * - T. G. Kolda and B. W. Bader. Tensor Decompositions and Applications. 
 * SIAM Review, Volume 51, Number 3, Pages 455-500, September 2009.
 * 
 */

#ifndef __VMML__T3_HOOI__HPP__
#define __VMML__T3_HOOI__HPP__


#include <vmmlib/t3_hosvd.hpp>
//#include <vmmlib/matrix_pseudoinverse.hpp>

namespace vmml
{
	
	template< size_t R1, size_t R2, size_t R3, size_t I1, size_t I2, size_t I3, typename T = float >
	class t3_hooi
	{
	public:    
		
		typedef tensor3< I1, I2, I3, T > t3_type;
		
		typedef tensor3< R1, R2, R3, T > t3_core_type;
		
		typedef matrix< I1, R1, T > u1_type;
		typedef matrix< I2, R2, T > u2_type;
		typedef matrix< I3, R3, T > u3_type;
		
		typedef matrix< R1, I1, T > u1_inv_type;
		typedef matrix< R2, I2, T > u2_inv_type;
		typedef matrix< R3, I3, T > u3_inv_type;

		/*	higher-order orthogonal iteration (HOOI) is a truncated HOSVD decompositions, i.e., the HOSVD components are of lower-ranks. An optimal rank-reduction is 
		 performed with an alternating least-squares (ALS) algorithm, which minimizes the error between the approximated and orignal tensor based on the Frobenius norm
		 see: De Lathauwer et al, 2000b; On the best rank-1 and rank-(RRR) approximation of higher-order tensors.
		 the HOOI can be computed based on (a) n-mode PCA, i.e., an eigenvalue decomposition on the covariance matrix of every mode's matriciziation, and 
		 (b) by performing a 2D SVD on the matricization of every mode. Matrix matricization means that a tensor I1xI2xI3 is unfolded/sliced into one matrix
		 with the dimensions I1xI2I3, which corresponds to a matrizitation alonge mode I1.
		 */
		static void als(  const t3_type& data_, u1_type& u1_, u2_type& u2_, u3_type& u3_, t3_core_type& core_ );

		
 		static void init( const t3_type& data_, u1_type& u1_, u2_type& u2_, u3_type& u3_ );
		static void init_random( const t3_type& data_, u1_type& u1_, u2_type& u2_, u3_type& u3_ );

		/* derive core
		 implemented accodring to core = data x_1 U1_pinv x_2 U2_pinv x_3 U3_pinv, 
		 where x_1 ... x_3 are n-mode products and U1_pinv ... U3_pinv are inverted basis matrices
		 the inversion is done with a matrix pseudoinverse computation
		 */
		static void derive_core(  const t3_type& data_, const u1_type& u1_, const u2_type& u2_, const u3_type& u3_, t3_core_type& core_ );
		//faster: but only if basis matrices are orthogonal
		static void derive_core_orthogonal_bases(  const t3_type& data_, const u1_type& u1_, const u2_type& u2_, const u3_type& u3_, t3_core_type& core_  );
		
		
	protected:
	
		template< size_t M, size_t N >
        static void fill_random_2d( int seed, matrix< M, N, T >& u );
				
        static void optimize_mode1( const t3_type& data_, const u2_type& u2_, const u3_type& u3_, tensor3< I1, R2, R3, T >& projection_ );
        static void optimize_mode2( const t3_type& data_, const u1_type& u1_, const u3_type& u3_, tensor3< R1, I2, R3, T >& projection_ );		
        static void optimize_mode3( const t3_type& data_, const u1_type& u1_, const u2_type& u2_, tensor3< R1, R2, I3, T >& projection_ );
		
	};//end class t3_hooi
	
#define VMML_HOOI_TEMPLATE_STRING        template< size_t R1, size_t R2, size_t R3, size_t I1, size_t I2, size_t I3, typename T >
#define VMML_HOOI_TEMPLATE_CLASSNAME     t3_hooi< R1, R2, R3, I1, I2, I3, T >

	
VMML_HOOI_TEMPLATE_STRING
void 
VMML_HOOI_TEMPLATE_CLASSNAME::init( const t3_type& data_, u1_type& u1_, u2_type& u2_, u3_type& u3_ )
{	
#if RAND_INIT
	init_random( data_, u1_, u2_, u3_ );
#else
	t3_hosvd< R1, R2, R3, I1, I2, I3, T >::apply_all( data_, u1_, u2_, u3_ );
#endif
}	
	

VMML_HOOI_TEMPLATE_STRING
void 
VMML_HOOI_TEMPLATE_CLASSNAME::init_random( const t3_type& data_, u1_type& u1_, u2_type& u2_, u3_type& u3_ )
{	
	int seed = time(NULL);
	fill_random_2d(seed, u1_ );
	fill_random_2d(rand(), u2_ );
	fill_random_2d(rand(), u3_ );
}	
	
VMML_HOOI_TEMPLATE_STRING
template< size_t M, size_t N >
void 
VMML_HOOI_TEMPLATE_CLASSNAME::fill_random_2d( int seed, matrix< M, N, T >& u)
{
	double fillValue = 0.0f;
	srand(seed);
	for( size_t row = 0; row < M; ++row )
	{
		for( size_t col = 0; col < N; ++col )
		{
			fillValue = rand();
			fillValue /= RAND_MAX;
			u.at( row, col ) = -1.0 + 2.0 * static_cast< double >( fillValue )  ;
		}
	}
}	
	
	
	
	
VMML_HOOI_TEMPLATE_STRING
void 
VMML_HOOI_TEMPLATE_CLASSNAME::als( const t3_type& data_, u1_type& u1_, u2_type& u2_, u3_type& u3_, t3_core_type& core_ )
{
	//intialize basis matrices
	init( data_, u1_, u2_, u3_ );
	
	//derve core from initialized matrices
	derive_core_orthogonal_bases( data_, u1_, u2_, u3_, core_ );
	
	//compute best rank-(R1, R2, R3) approximation (Lathauwer et al., 2000b)
	t3_type approximated_data;
	approximated_data.full_tensor3_matrix_multiplication( core_, u1_, u2_, u3_);
	
	double f_norm = approximated_data.frobenius_norm();
	double max_f_norm = data_.frobenius_norm();
	double normresidual  = sqrt( (max_f_norm * max_f_norm) - (f_norm * f_norm));
	double fit = 0;
	if (max_f_norm != 0 ) {
		fit = 1 - (normresidual / max_f_norm);
	} else { 
		fit = 1;
	}
	
	double fitchange = fit;
	double fitold = fit;
	double fitchange_tolerance = 1.0e-4;
	
	tensor3< I1, R2, R3, T > projection1; 
	tensor3< R1, I2, R3, T > projection2; 
	tensor3< R1, R2, I3, T > projection3; 
	
#if TUCKER_LOG
	std::cout << "HOOI ALS (for tensor3) " << std::endl 
	<< "initial fit: " << fit  << ", "
	<< "frobenius norm original: " << max_f_norm << std::endl;
#endif	
	size_t i = 0;
	size_t max_iterations = 10;
	while( (fitchange >= fitchange_tolerance) && (i < max_iterations) )
	{
		fitold = fit;
		
		//optimize modes
		optimize_mode1( data_, u2_, u3_, projection1 );
		t3_hosvd< R1, R2, R3, I1, R2, R3, T >::apply_mode1( projection1, u1_ );
		
		optimize_mode2( data_, u1_, u3_, projection2 );
		t3_hosvd< R1, R2, R3, R1, I2, R3, T >::apply_mode2( projection2, u2_ );
		
		optimize_mode3( data_, u1_, u2_, projection3 );
		t3_hosvd< R1, R2, R3, R1, R2, I3, T >::apply_mode3( projection3, u3_ );
		
		core_.multiply_horizontal_bwd( projection3, transpose( u3_ ) );
		f_norm = core_.frobenius_norm();
		normresidual  = sqrt( max_f_norm * max_f_norm - f_norm * f_norm);
		fit = 1 - (normresidual / max_f_norm);
		fitchange = fabs(fitold - fit);
		
#if TUCKER_LOG
		std::cout << "iteration '" << i << "', fit: " << fit 
		<< ", fitdelta: " << fitchange 
		<< ", frobenius norm: " << f_norm << std::endl;		
#endif
		++i;
	}
}	



VMML_HOOI_TEMPLATE_STRING
void 
VMML_HOOI_TEMPLATE_CLASSNAME::optimize_mode1( const t3_type& data_, const u2_type& u2_, const u3_type& u3_, tensor3< I1, R2, R3, T >& projection_ )
{
	u2_inv_type* u2_inv = new u2_inv_type();
	*u2_inv = transpose( u2_ );
	u3_inv_type* u3_inv = new u3_inv_type();
	*u3_inv = transpose( u3_ );
	
	//backward cyclic matricization/unfolding (after Lathauwer et al., 2000a)
	tensor3< I1, R2, I3, T >* tmp  = new tensor3< I1, R2, I3, T >();
	tmp->multiply_frontal_bwd( data_, *u2_inv );
	projection_.multiply_horizontal_bwd( *tmp, *u3_inv );
	
	delete u2_inv;
	delete u3_inv;
	delete tmp;
}


VMML_HOOI_TEMPLATE_STRING
void 
VMML_HOOI_TEMPLATE_CLASSNAME::optimize_mode2( const t3_type& data_, const u1_type& u1_, const u3_type& u3_, tensor3< R1, I2, R3, T >& projection_ )
{
	u1_inv_type* u1_inv = new u1_inv_type();
	*u1_inv = transpose( u1_ );
	u3_inv_type* u3_inv = new u3_inv_type();
	*u3_inv = transpose( u3_ );
	
	//backward cyclic matricization (after Lathauwer et al., 2000a)
	tensor3< R1, I2, I3, T >* tmp = new tensor3< R1, I2, I3, T >();
	tmp->multiply_lateral_bwd( data_, *u1_inv );
	projection_.multiply_horizontal_bwd( *tmp, *u3_inv );
	
	delete u1_inv;
	delete u3_inv;
	delete tmp;
}


VMML_HOOI_TEMPLATE_STRING
void 
VMML_HOOI_TEMPLATE_CLASSNAME::optimize_mode3( const t3_type& data_, const u1_type& u1_, const u2_type& u2_, tensor3< R1, R2, I3, T >& projection_ )
{
	u1_inv_type* u1_inv = new u1_inv_type();
	*u1_inv = transpose( u1_ );
	u2_inv_type* u2_inv = new u2_inv_type();
	*u2_inv = transpose( u2_ );
	
	//backward cyclic matricization (after Lathauwer et al., 2000a)
	tensor3< R1, I2, I3, T >* tmp = new tensor3< R1, I2, I3, T >();
	tmp->multiply_lateral_bwd( data_, *u1_inv );
	projection_.multiply_frontal_bwd( *tmp, *u2_inv );
	
	delete u1_inv;
	delete u2_inv;
	delete tmp;
}
	
	
	
VMML_HOOI_TEMPLATE_STRING
void 
VMML_HOOI_TEMPLATE_CLASSNAME::derive_core_orthogonal_bases( const t3_type& data_, const u1_type& u1_, const u2_type& u2_, const u3_type& u3_, t3_core_type& core_ )
{
	u1_inv_type* u1_inv = new u1_inv_type();
	*u1_inv = transpose( u1_ );
	u2_inv_type* u2_inv = new u2_inv_type();
	*u2_inv = transpose( u2_ );
	u3_inv_type* u3_inv = new u3_inv_type();
	*u3_inv = transpose( u3_ );
	
	core_.full_tensor3_matrix_multiplication( data_, *u1_inv, *u2_inv, *u3_inv );
	
	delete u1_inv;
	delete u2_inv;
	delete u3_inv;
}


VMML_HOOI_TEMPLATE_STRING
void 
VMML_HOOI_TEMPLATE_CLASSNAME::derive_core( const t3_type& data_, const u1_type& u1_, const u2_type& u2_, const u3_type& u3_, t3_core_type& core_ )
{
	
#if 0
	//compute pseudo inverse for matrices u1-u3
	u1_comp_type u1_pinv_t ;
	u2_comp_type u2_pinv_t ;
	u3_comp_type u3_pinv_t ;
	
	
	compute_pseudoinverse<  u1_type > compute_pinv_u1;
	compute_pinv_u1( *_u1_comp, u1_pinv_t );
	compute_pseudoinverse<  u2_type > compute_pinv_u2;
	compute_pinv_u2( *_u2_comp, u2_pinv_t );	
	compute_pseudoinverse<  u3_type > compute_pinv_u3;
	compute_pinv_u3( *_u3_comp, u3_pinv_t );
	
	u1_inv_type* u1_pinv = new u1_inv_type();
	*u1_pinv = transpose( u1_pinv_t );
	u2_inv_type* u2_pinv = new u2_inv_type();
	*u2_pinv = transpose( u2_pinv_t );
	u3_inv_type* u3_pinv = new u3_inv_type();
	*u3_pinv = transpose( u3_pinv_t );
	
	t3_comp_type data;
	datacast_from( data_ );
	_core_comp.full_tensor3_matrix_multiplication( data, *u1_pinv, *u2_pinv, *u3_pinv );
	
	delete u1_pinv;
	delete u2_pinv;
	delete u3_pinv;
	
#else
	//previous version of compute core	
	for( size_t r3 = 0; r3 < R3; ++r3 )
	{
		for( size_t r1 = 0; r1 < R1; ++r1 )
		{
			for( size_t r2 = 0; r2 < R2; ++r2 )
			{
				float_t sum_i1_i2_i3 = 0.0;
				for( size_t i3 = 0; i3 < I3; ++i3 )
				{
					for( size_t i1 = 0; i1 < I1; ++i1 )
					{
						for( size_t i2 = 0; i2 < I2; ++i2 )
						{
							sum_i1_i2_i3 += u1_.at( i1, r1 ) * u2_.at( i2, r2 ) * u3_.at( i3, r3 ) * T(data_.at( i1, i2, i3 ));
						}
					}
				}
				core_.at( r1, r2, r3 ) = sum_i1_i2_i3;
			}
		}
	}
	
#endif
}

	
	
	
}//end vmml namespace

#endif