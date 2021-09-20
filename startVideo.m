clear;
% for ii = 20:-1:1
%     disp(ii);
%     pause(1);
% end
width = 3392;
height = 3392;
offset_x = 672;
offset_y = 12;
rate = 50;
gain = 4;
gain2 = 800;
Aoffset = 0;
exposition = 19992;
frames = 50*60*5;
dt = 60*41;
numbers = 1%76+3600*2/dt;
save('info.mat')
curdir = cd;

% for nn = (0:numbers-1)
%     disp(nn)
%     shift = width*height*frames*nn;
%     tic;
%     %    cd '/media/fill/cube/Develop/VisionPointApi/';
%     sys_str = num2str([width height offset_x offset_y rate frames gain gain2 Aoffset exposition shift],...
%      'sudo ./KYFGLib_to_ssd -w %d -h %d -x %d -y %d -r %d -o /dev/nvme0n1 -m %d -g %d -G %d -a %d -e %d -s %d');
%     disp(sys_str)
%     system(sys_str);
%     %    cd(curdir)
%     disp(num2str(dt-toc, 'wait %0.2fs.'))
%     while(toc<dt); end
% 
% end

%patho = '/media/fill/cube/video/CubeExp/Test/';
nn = 0;
patho = '/media/cube/video/CubeExp/RotationMix/';
mkdir(patho)
%patho = '/media/fill/cube/video/CubeExp/TestPump/';
% return
copyfile('info.mat', patho)
name = 'pumpMix_noRot_2.dat';
% name = 'rotDecay2_2hup_2hdown.dat';
%name = 'test_no_rot_pump8_60deg.dat';
sys_str = sprintf('sudo dd if=/dev/nvme0n1 of=%s bs=%dc count=%d status=progress', [patho name], width*height, frames*(nn+1))
disp(sys_str)
system(sys_str);

%A = loadFrame('test.dat', height, width, 15); 
%imshow(A(1000:1100, 1000:1100))
