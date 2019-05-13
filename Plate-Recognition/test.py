import time
import torch
from torch.utils.data import DataLoader
from torchvision import transforms
from dataset import PlateData, Resize, ToTensor
from resnet import resnet34
from utils import vec2txt


root_dir = '../../final/train_data/train-data'
tsfm = transforms.Compose([Resize((70, 356)), ToTensor()])
testset = PlateData('data/train-data-label.txt', root_dir, transform=tsfm)
testloader = DataLoader(testset, batch_size=100)

device = 'cuda'
start = time.time()

# prepare model
model = resnet34(pretrained=False)
model.load_state_dict(torch.load('./model-ckpt/best_acc_state.pth'), strict=False)
model.to(device)

f = open('predicted.txt', 'w')
f.write('    GT          pred     prec\n')
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
        predicted_t = predicted
        predicted = (predicted - labels).abs()
        predicted = predicted.sum(1)
        predicted = predicted == 0
        correct_plate += predicted.sum().item()

        # record per sample
        labels_np = labels.cpu().numpy()
        predicted_np = predicted_t.cpu().numpy()
        for i in range(sample_num):
            ground_truth = vec2txt(labels_np[i])
            pred = vec2txt(predicted_np[i])
            correct = (labels_np[i] == predicted_np[i]).sum()
            line = '{0}   {1}   {2}/9\n'.format(ground_truth, pred, correct)
            f.write(line)

        print('process {0}'.format(total_plate))

f.close()
print(test_acc, total_plate)
avg_acc = sum(test_acc) / (total_plate * 9)
comp_corr_plate_acc = correct_plate / total_plate
for i in range(9):
    test_acc[i] /= total_plate

print(test_acc)
print('char_avg_acc: {0:.4f}, complete_plate_acc: {1:.4f}'.format(avg_acc, comp_corr_plate_acc))
duration = time.time() - start
print('Done, total time used: {0:.2f} s, per img time used: {1:.4f} s'.format(duration, duration/total_plate))


