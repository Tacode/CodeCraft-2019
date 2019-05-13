import numpy as np

Chinese2CharIdx = {'深': 0, '秦': 1, '京': 2, '海': 3, '成': 4, '南': 5, '杭': 6, '苏': 7, '松': 8}
CharIdx2Chinese = {0: '深', 1: '秦', 2: '京', 3: '海', 4: '成', 5: '南', 6: '杭', 7: '苏', 8: '松'}


# 把txt的车牌变换成9位向量
def txt2vec(txt):
    """
    Args:
        txt(str): 长度为9个字符的车牌str，如'深Y2SC1L8S'

    Returns: 
        vec(np.ndarray): 对label每个字符进行编码之后长度为9的向量
    """
    assert len(txt) == 9
    vec = np.zeros(9, dtype=np.uint8)
    vec[0] = Chinese2CharIdx[txt[0]]
    vec[1] = ord(txt[1]) - ord('A')
    for i in range(2, 9):
        char = txt[i]
        if char.isalpha():
            vec[i] = ord(char) - ord('A')
        else:
            vec[i] = ord(char) - ord('0') + 26
    
    return vec

# 把9位向量转换成车牌
def vec2txt(vec):
    """
    Args:
        vec(np.ndarray): 对label每个字符进行编码之后长度为9的向量

    Returns: 
        txt(str): 长度为9个字符的车牌str，如'深Y2SC1L8S'
    """
    txt = []
    # 单独处理前两个字符， 第一个中文和第二个大写字母
    txt.append(str(vec[0]))
    txt.append(chr(vec[1] + ord('A')))
    for i in range(2, 9):
        idx = vec[i]
        if idx <= 25: # 0-25 对应的是大写字母
            txt.append(chr(idx + ord('A')))
        else: # 26-35 对应的是数字
            txt.append(chr(idx - 26 + ord('0')))
    
    return ''.join(txt)




