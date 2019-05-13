import os
import torch
import numpy as np
from PIL import Image
from skimage import io, transform
from skimage.util import random_noise, img_as_ubyte
from torch.utils.data import Dataset
from utils import txt2vec


class PlateData(Dataset):
    def __init__(self, data_file, root_dir, transform=None):
        f = open(data_file)
        lines = f.readlines()
        f.close()
        self.lines = lines
        self.root_dir = root_dir
        self.transform = transform
    
    def __len__(self):
        return len(self.lines)

    def __getitem__(self, idx):
        label_txt, img_name = self.lines[idx].split(',')
        img_name = img_name.strip()
        img_path = os.path.join(self.root_dir, img_name)
        img = io.imread(img_path)
        label = txt2vec(label_txt)
        if self.transform:
            img = self.transform(img)
        return img, label

# 使用skimage.util.random_noise 添加噪声
class RandomNoise():
    def __init__(self,mode='gaussian', seed=None, clip=True, **kwargs):
        self.mode = mode
        self.seed = seed
        self.clip = clip
        self.kwargs = kwargs

    def __call__(self, img):
        """
            Args: 
                img(PIL.Image): PIL 图片
            Returns:
                img(PIL.Image)
        """
        img = np.asarray(img)
        img = random_noise(img, self.mode, self.seed, self.clip, **self.kwargs)
        img = img_as_ubyte(img)
        img = Image.fromarray(img)
        return img

class Resize():
    def __init__(self, output_size):
        self.output_size = output_size

    def __call__(self, img):
        h, w = img.shape[:2]
        new_h, new_w = self.output_size
        if h != new_h or w != new_w:
            img = transform.resize(img, (new_h, new_w))
        img = img.astype(np.float32)
        return img

class ToTensor():
    """ Convert ndarray to Tensors """
    def __call__(self, img):
        img = img.transpose(2, 0, 1)    # (H, W, C) -> (C, H, W)
        img = torch.from_numpy(img)
        img = img / 255
        return img
    

