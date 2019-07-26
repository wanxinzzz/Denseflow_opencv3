from __future__ import print_function, division
import os
import sys
import subprocess

def class_process(dir_path, dst_dir_path, class_name):
  class_path = os.path.join(dir_path, class_name)
  if not os.path.isdir(class_path):
    return

  dst_class_path = os.path.join(dst_dir_path, class_name)
  if not os.path.exists(dst_class_path):
    os.mkdir(dst_class_path)

  class_videos = os.listdir(class_path)
  class_videos.sort()
  for file_name in class_videos:
    if ('.avi' not in file_name) and ('.mp4' not in file_name):
      continue
    name, ext = os.path.splitext(file_name)
    dst_directory_path = os.path.join(dst_class_path, name)

    video_file_path = os.path.join(class_path, file_name)
    try:
      if os.path.exists(dst_directory_path):
        if not os.path.exists(os.path.join(dst_directory_path, 'img_00001.jpg')):
          subprocess.call('rd  \"{}\"'.format(dst_directory_path), shell=True)
          print('remove {}'.format(dst_directory_path))
          os.mkdir(dst_directory_path)
        else:
          continue
      else:
        os.mkdir(dst_directory_path)
    except:
      print(dst_directory_path)
      continue
    cmd = './get_flow_gpu -f={} -x={}/flowx -y={}/flowy -i={}/img -b=20 -t=1 -d=0 -s=1 -h=0 -w=0'.format(video_file_path, 
                                                                  dst_directory_path, dst_directory_path, dst_directory_path)
    print(cmd)
    subprocess.call(cmd, shell=True)
    print('\n')

if __name__=="__main__":
  dir_path = sys.argv[1]
  dst_dir_path = sys.argv[2]

  if not os.path.exists(dst_dir_path):
    os.mkdir(dst_dir_path)

  video_classes = os.listdir(dir_path)
  video_classes.sort()

  for class_name in video_classes:
    class_process(dir_path, dst_dir_path, class_name)
