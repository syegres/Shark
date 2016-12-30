#define BOOST_TEST_MODULE LinAlg_BLAS_GPU_MatrixAssign
#define BOOST_COMPUTE_DEBUG_KERNEL_COMPILATION
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <shark/Core/Shark.h>
#include <shark/LinAlg/BLAS/blas.h>
#include <shark/LinAlg/BLAS/gpu/matrix.hpp>
#include <shark/LinAlg/BLAS/gpu/copy.hpp>

using namespace shark;

template<class M1, class M2>
void checkMatrixEqual(M1 const& m1_gpu, M2 const& m2_gpu){
	BOOST_REQUIRE_EQUAL(m1_gpu.size1(),m2_gpu.size1());
	BOOST_REQUIRE_EQUAL(m1_gpu.size2(),m2_gpu.size2());
	
	blas::matrix<float> m1 = copy_to_cpu(m1_gpu);
	blas::matrix<float> m2 = copy_to_cpu(m2_gpu);
	for(std::size_t i = 0; i != m2.size1(); ++i){
		for(std::size_t j = 0; j != m2.size2(); ++j){
			BOOST_CHECK_EQUAL(m1(i,j),m2(i,j));
		}
	}
}

BOOST_AUTO_TEST_SUITE (LinAlg_BLAS_gpu_matrix_assign)

BOOST_AUTO_TEST_CASE( LinAlg_BLAS_Matrix_Assign_Dense ){
	std::cout<<"testing dense-dense assignment"<<std::endl;
	blas::matrix<float> source_cpu(100,237);
	blas::matrix<float> target_cpu(100,237);
	blas::matrix<float> result_add_cpu(100,237);
	blas::matrix<float> result_add_scalar_cpu(100,237);
	float scalar = 10;
	for(std::size_t i = 0; i != 100; ++i){
		for(std::size_t j = 0; j != 237; ++j){
			source_cpu(i,j) = 2*i+1+0.3*j;
			target_cpu(i,j) = 3*i+2+0.3*j;
			result_add_cpu(i,j) = source_cpu(i,j) + target_cpu(i,j);
			result_add_scalar_cpu(i,j) = target_cpu(i,j) + scalar;
		}
	}
	blas::gpu::matrix<float, blas::row_major> source = blas::gpu::copy_to_gpu(source_cpu);
	blas::gpu::matrix<float, blas::column_major> source_cm = blas::gpu::copy_to_gpu(source_cpu);
	blas::gpu::matrix<float> result_add = blas::gpu::copy_to_gpu(result_add_cpu);
	blas::gpu::matrix<float> result_add_scalar = blas::gpu::copy_to_gpu(result_add_scalar_cpu);
	{
		std::cout<<"testing direct assignment row-row"<<std::endl;
		blas::gpu::matrix<float> target = blas::gpu::copy_to_gpu(target_cpu);
		blas::kernels::assign(target,source);
		checkMatrixEqual(target,source);
	}
	{
		std::cout<<"testing functor assignment row-row"<<std::endl;
		blas::gpu::matrix<float> target = blas::gpu::copy_to_gpu(target_cpu);
		blas::kernels::assign<blas::device_traits<blas::gpu_tag>::add<float> >(target,source);
		checkMatrixEqual(target,result_add);
	}
	{
		std::cout<<"testing direct assignment row-column"<<std::endl;
		blas::gpu::matrix<float> target = blas::gpu::copy_to_gpu(target_cpu);
		blas::kernels::assign(target,source_cm);
		checkMatrixEqual(target,source_cm);
	}
	{
		std::cout<<"testing functor assignment row-column"<<std::endl;
		blas::gpu::matrix<float> target = blas::gpu::copy_to_gpu(target_cpu);
		blas::kernels::assign<blas::device_traits<blas::gpu_tag>::add<float> >(target,source_cm);
		checkMatrixEqual(target,result_add);
	}
	{
		std::cout<<"testing functor scalar assignment"<<std::endl;
		blas::gpu::matrix<float> target = blas::gpu::copy_to_gpu(target_cpu);
		blas::kernels::assign<blas::device_traits<blas::gpu_tag>::add<float> >(target,scalar);
		target.queue().finish();
		checkMatrixEqual(target,result_add_scalar);
	}
	
}

BOOST_AUTO_TEST_SUITE_END()