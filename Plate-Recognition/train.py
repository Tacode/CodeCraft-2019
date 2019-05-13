import os
import time
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader
from torchvision import transforms
from resnet import resnet34
from dataset import PlateData, Resize, ToTensor, RandomNoise
from model import Model

# prepare data
root_dir = '../../final/train_data/train-data'
tsfm = transforms.Compose([
    Resize((70, 356)),
    ToTensor()
])
trainset = PlateData('data/all-data.txt', root_dir, transform=tsfm)
testset = PlateData('data/all-data.txt', root_dir, transform=tsfm)
trainloader = DataLoader(trainset, batch_size=8, shuffle=True)
test_trainloader = DataLoader(trainset, batch_size=50)
testloader = DataLoader(testset, batch_size=100)

device = 'cuda'

# prepare model
model = resnet34(pretrained=False)
model.load_state_dict(torch.load('./model-ckpt/resnet34-best.pth'), strict=True)

# model = Model()
model.to(device)

# define loss function and optimizer
criterion = nn.CrossEntropyLoss()
optimizer = optim.SGD(model.parameters(), lr=0.004)
scheduler = optim.lr_scheduler.MultiStepLR(optimizer, [10, 20, 30, 40], gamma=0.5)

start = time.time()
best_acc = 0

# main train loop
for epoch in range(1, 51):
    scheduler.step()
    epoch_start = time.time()
    running_loss = 0.0
    for step, data in enumerate(trainloader, 1):
        # 1. get inputs
        imgs, labels = data
        imgs = imgs.to(device)
        labels = labels.to(device, dtype=torch.long)

        # 2. zero parameters gradients
        optimizer.zero_grad()

        # 3. forward + backward + optimize
        outputs = model(imgs)
        for i, out in enumerate(outputs):
            loss = criterion(out, labels[:, i])
            retain_graph = False if i == 8 else True
            loss.backward(retain_graph=retain_graph)
            running_loss += loss.item()
        optimizer.step()
        if step % 100 == 0:
            print('[%d, %4d] loss: %.4f'%(epoch, step, running_loss/100))
            running_loss = 0.0

    # test accuracy in test data
    total_plate, correct_plate = 0, 0
    test_acc = [0 for _ in range(9)]
    with torch.no_grad():
        for step, data in enumerate(testloader, 1):
            imgs, labels = data
            imgs = imgs.to(device)
            labels = labels.to(device, dtype=torch.long)
            sample_num = labels.size(0)
            total_plate += labels.size(0)

            outputs = model(imgs)
            predicted = torch.zeros((labels.shape), device=device, dtype=torch.long)
            for i, out in enumerate(outputs):
                pred_idx = out.argmax(dim=1).to(torch.long)
                predicted[torch.arange(sample_num), i] = pred_idx
                correct = (pred_idx == labels[:, i]).sum().item()
                test_acc[i] += correct

            predicted = (predicted - labels).abs()
            predicted = predicted.sum(1)
            predicted = predicted == 0
            correct_plate += predicted.sum().item()
    print(test_acc, total_plate)
    avg_acc = sum(test_acc) / (total_plate * 9)
    comp_corr_plate_acc = correct_plate / total_plate
    if avg_acc > best_acc:
        best_acc = avg_acc
        best_state = model.state_dict()
        torch.save(best_state, './model-ckpt/best_acc_state.pth')
        print('save best acc {0:.4f} state_dict at epoch: {1}...'.format(best_acc, epoch))
    for i in range(9):
        test_acc[i] /= total_plate
    duration = time.time() - epoch_start
    print(test_acc)
    print('{0:.4f} {1:.4f} {2:.4f}'.format(avg_acc, best_acc, comp_corr_plate_acc))
    print('epoch {0}, time used: {1:.2f} s'.format(epoch, duration))

duration = time.time() - start
print('Done! total time used: {0:.2f} min'.format(duration/60))

