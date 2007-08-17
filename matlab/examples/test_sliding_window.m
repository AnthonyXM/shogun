%generate some toy data
acgt='ACGT';
dat={acgt([1*ones(1,10) 2*ones(1,10) 3*ones(1,10) 4*ones(1,10) 1])};
sg('send_command', 'loglevel ALL');
sg('set_features', 'TRAIN', dat, 'DNA');
sg('send_command', 'slide_window TRAIN 5 1');

f=sg('get_features', 'TRAIN')


sg('set_features', 'TRAIN', dat, 'DNA');
sg('from_position_list','TRAIN', 5,[0,1,2,5,15,25,30,36]);

f=sg('get_features', 'TRAIN')

sg('set_features', 'TEST', dat, 'DNA');
sg('from_position_list','TEST', 5,[0,1,2,5,15,25,30,36]);

ft=sg('get_features', 'TEST')

C=1;
order=20;
order_com=5;
max_mismatch=0;
len=200;
shift=0;
num=100;
num_test=5000;
cache=10;
normalize=1;
mkl_stepsize=1;
block=0;
single_degree=-1;
sg('send_command', sprintf( 'set_kernel WEIGHTEDDEGREE STRING %i %i %i %i %i %i %i', cache, order, max_mismatch, normalize, mkl_stepsize, block, single_degree) );
sg('send_command','init_kernel TRAIN')
km=sg('get_kernel_matrix')

sg('send_command','clean_features TRAIN');
sg('send_command','clean_features TEST');
sg('set_features', 'TRAIN', dat, 'DNA');
sg('from_position_list','TRAIN', 5, [0,1,2,5,15,25,30]+5);
sg('set_features', 'TRAIN', dat, 'DNA');
sg('from_position_list','TRAIN', 5, [0,1,2,5,15,25]+9);
sg('send_command','clean_features TRAIN');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
rand('seed',17);
len=50;
num_train=100;
total_test_len=778;
order=2;
num_test=total_test_len-len-1;
use_sign=0;
normalization='NO';

acgt='ACGT';
traindat=acgt(ceil(4*rand(len,num_train)));
trainlab=[-ones(1,num_train/2),ones(1,num_train/2)];

testdat=acgt(ceil(4*rand(1,total_test_len)));
testlab=[-ones(1,num_test/2),ones(1,num_test/2)];

testdat1=char(zeros(len,num_test));
for i=order:(total_test_len-len+1),
	testdat1(:,i-order+1)=testdat(i:(i+(len-1)));
end

%train svm
sg('send_command', 'use_linadd 1' );
sg('set_features', 'TRAIN', traindat, 'DNA');
sg('set_labels', 'TRAIN', trainlab);

sg('send_command', sprintf('convert TRAIN STRING CHAR STRING WORD %i %i', order, order-1));
sg('send_command', 'add_preproc SORTWORDSTRING') ;
sg('send_command', 'attach_preproc TRAIN') ;
sg('send_command', sprintf('set_kernel COMMSTRING WORD %d %d %s',cache, use_sign, normalization));

sg('send_command', 'init_kernel TRAIN');
sg('send_command', 'new_svm LIGHT');
sg('send_command', sprintf('c %f',C));
sg('send_command', 'svm_train');
sg('send_command', 'init_kernel_optimization');

%evaluate svm on test data
sg('set_features', 'TEST', testdat1, 'DNA');
sg('send_command', sprintf('convert TEST STRING CHAR STRING WORD %i %i', order, order-1));
sg('send_command', 'attach_preproc TEST') ;
sg('set_labels', 'TEST', testlab);
sg('send_command', 'init_kernel TEST');
out2=sg('svm_classify');
f2=sg('get_features','TEST');

%evaluate svm on test data
sg('set_features', 'TEST', testdat1, 'DNA');
sg('send_command', sprintf('convert TEST STRING CHAR STRING WORD %i %i', order, order-1));
sg('set_labels', 'TEST', testlab);
sg('send_command', 'init_kernel TEST');
out1=sg('svm_classify');
f1=sg('get_features','TEST');

%evaluate svm on test data
sg('set_features', 'TEST', testdat', 'DNA');
sg('send_command', sprintf('convert TEST STRING CHAR STRING WORD %i %i', order, order-1));
sg('send_command', 'slide_window TEST 50 1');
%sg('from_position_list','TEST', len, 0:(total_test_len-len-order+1));
sg('set_labels', 'TEST', testlab);
sg('send_command', 'init_kernel TEST');
out=sg('svm_classify');
f=sg('get_features','TEST');

max(abs(out(:)-out1(:)))
max(abs(out1(:)-out2(:)))
