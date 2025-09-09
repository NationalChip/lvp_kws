# -*- coding: utf-8 -*-
from __future__ import unicode_literals
import re, sys, os
#
absolute_path = os.path.dirname(os.path.abspath(__file__))
try:
    sys.path.append(absolute_path)
    from modules.common.gxPinYing import pypinyin, pinyin
except:
    from modules.common.gxPinYing import pypinyin, pinyin

if os.path.exists(os.path.join(absolute_path, 'units/units.txt')):
    units_file = os.path.join(absolute_path, 'units/units.txt')
    units_tone_file = os.path.join(absolute_path, 'units/units_tone.txt')
    units_syl_file = os.path.join(absolute_path, 'units/units_yinjie.txt')
else:
    units_file = 'modules/common/units/units.txt'
    units_tone_file = 'modules/common/units/units_tone.txt'
    units_syl_file = 'modules/common/units/units_yinjie.txt'



def is_all_chinese(str):
    if re.match(r'^[\u4e00-\u9fff]+$', str):
        return True
    else:
        return False


def merge_adjacent(s):
    result = []
    skip_next = False
    words = s.split()

    for i in range(len(words) - 1):
        if skip_next:
            skip_next = False
            continue
        if words[i] == 'sil' or words[i + 1] == 'sil':
            result.append(words[i])
        elif words[i] == 'spn' or words[i + 1] == 'spn':
            result.append(words[i])
        else:
            result.append(words[i] + words[i + 1])
            skip_next = True
    if not skip_next:
        result.append(words[-1])

    return ' '.join(result)

def init_dict(dict_file):
    phone_to_id = {}
    with open(dict_file,'r') as f:
        for line in f.readlines():
            line=line.replace('\n','')
            splits=line.split(' ')
            phone_to_id[splits[0]]=int(splits[1])
        id_to_phone = dict(zip(phone_to_id.values(),phone_to_id.keys()))

    return phone_to_id, id_to_phone


def oov(tem_shenmu,tem_yunmu,dic,py_shenmu,py_yunmu,txt):
    shenmu = tem_shenmu
    yunmu = tem_yunmu
    if shenmu not in dic:
        shenmu = 'sil'
    if yunmu not in dic:
        yunmu = 'sil'
    return shenmu,yunmu


def change_yunmu(yunmu):
    if yunmu[-1] != '1' and yunmu[-1] != '2' and yunmu[-1] != '3' and yunmu[-1] != '4':
        yunmu = yunmu+'0'

    return yunmu


def get_sym_labels(txt, is_tone = False):
    char = re.sub(r'\*\*','',txt)
    char = re.sub(r'\*','',char)
    char = re.sub(r'<FIL/>','',char)
    char = re.sub(r'<NON/>','',char)
    char = re.sub(r'<NPS/>','',char)
    char = re.sub(r'<SPK/>','',char)
    char = re.sub(r'<STA/>','',char)
    char = re.sub(r'<UNK>','',char)
    char = re.sub(r'[\s+\.\!\/_,$%^*(+\"\']+|[+——！，。？?、~@#￥%……&*（）]', '',char)
    char = re.sub(r'   ','',char)
    char = re.sub(r'  ','',char)
    txt = re.sub(r' ','',char)

#    print txt
    unicode_txt = txt.replace(' ','')
#    print shengyunmu
    py_shenmu = pinyin(unicode_txt, style=pypinyin.INITIALS)
    if is_tone == True:
        dictinary_char_to_label, dictinary_label_to_char = init_dict(units_tone_file)
        py_yunmu = pinyin(unicode_txt, style=pypinyin.FINALS_TONE3)
    if is_tone == False:
        dictinary_char_to_label,dictinary_label_to_char = init_dict(units_file)
        py_yunmu = pinyin(unicode_txt, style=pypinyin.FINALS)
    labels =[]
    length = len(py_shenmu)
    for i in range(length):
        #mean are normal pinyin
        if is_tone == True:
            py_yunmu[i][0] = change_yunmu(py_yunmu[i][0])
        if py_shenmu[i][0] != "":
            if py_shenmu[i][0] in dictinary_char_to_label and py_yunmu[i][0] in dictinary_char_to_label:
                labels = labels + [dictinary_char_to_label[py_shenmu[i][0]]] + [dictinary_char_to_label[py_yunmu[i][0]]]

            else: #oov record
                tem_shenmu = py_shenmu[i][0]
                tem_yunmu = py_yunmu[i][0]
                tem_shenmu,tem_yunmu = oov(tem_shenmu,tem_yunmu,dictinary_char_to_label,py_shenmu,py_yunmu,txt)
                labels = labels + [dictinary_char_to_label[tem_shenmu]] + [dictinary_char_to_label[tem_yunmu]]

        else:
                #pdb.set_trace()
            if py_yunmu[i][0] in dictinary_char_to_label:
                tem_shenmu = py_yunmu[i][0][0] + py_yunmu[i][0][0]
                tem_shenmu,tem_yunmu = oov(tem_shenmu,py_yunmu[i][0],dictinary_char_to_label,py_shenmu,py_yunmu,txt)
                #add labels
                labels = labels + [dictinary_char_to_label[tem_shenmu]] + [dictinary_char_to_label[tem_yunmu]]

            elif py_yunmu[i][0][0] == "i" or py_yunmu[i][0][0] == "u":
                tem_shenmu = py_yunmu[i][0][0] + py_yunmu[i][0][0]
                tem_yunmu = py_yunmu[i][0][1:]
                tem_shenmu,tem_yunmu = oov(tem_shenmu,tem_yunmu,dictinary_char_to_label,py_shenmu,py_yunmu,txt)
                #add labels
                labels = labels + [dictinary_char_to_label[tem_shenmu]] + [dictinary_char_to_label[tem_yunmu]]

            elif py_yunmu[i][0][:-1] == "n":
                tem_shenmu = "ee"
                tem_yunmu = "en"+py_yunmu[i][0][-1]
                #add labels
                labels = labels + [dictinary_char_to_label[tem_shenmu]] + [dictinary_char_to_label[tem_yunmu]]

            else:
                print ("fuck!")
                tem_shenmu,tem_yunmu = oov(py_shenmu[i][0],py_yunmu[i][0],dictinary_char_to_label,py_shenmu,py_yunmu,txt)
                #add labels
                labels = labels + [dictinary_char_to_label[tem_shenmu]] + [dictinary_char_to_label[tem_yunmu]]

    return labels


def id_to_char(id_list, is_tone = False):
    char_list = []
    i = 0
    if is_tone == True:
        dictinary_char_to_label, dictinary_label_to_char = init_dict(units_tone_file)
    if is_tone == False:
        dictinary_char_to_label, dictinary_label_to_char = init_dict(units_file)

    for index,item in enumerate(id_list):
        if index==0 and item>0:
            char_list.append(dictinary_label_to_char[item])
            if item>0 :
                i = i + 1
            else:
                char_list.append(' ')
        else:
            if item>0 :
               if item > 0:
                   char_list.append(dictinary_label_to_char[item])
                   i = i + 1
                   if i%2 == 0:
                       char_list.append(' ')
               else:
                   char_list.append(' ')
                   char_list.append(dictinary_label_to_char[item])
                   char_list.append(' ')

    return ' '.join(char_list)


def id_to_char_2d(id_list_2d):
    char_list=[]
    for id_list in id_list_2d:
        char_list.append(id_to_char(id_list))
    return char_list


def get_yinjie_label(text):
    syl_dict, _ = init_dict(units_syl_file)
    ids = get_sym_labels(text.strip())

    phones = id_to_char(ids).replace('  ', ' ')
    syl_phones = merge_adjacent(phones)

    ids_tone = get_sym_labels(text.strip(), is_tone = True)
    phones_tone = id_to_char(ids_tone, is_tone = True).replace('  ', ' ')

    label_id = []
    for s in syl_phones.strip().split():
        if s in syl_dict:
            label_id.append(syl_dict[s])
        else:
            label_id.append(syl_dict['spn'])


    return phones, ids, syl_phones, label_id, phones_tone, ids_tone


if __name__=="__main__":
    if len(sys.argv) != 2:
        print ("ERROR *************")
        print ("please input cmd file")
        print ("Usage:{0} <cmd.txt>".format(sys.argv[0]))

    cmd_file = sys.argv[1]
    with open(cmd_file,"r") as f:
        for line in f.readlines():
            cmd = line.strip()

            assert is_all_chinese(cmd), '指令词包含非中文字符, 请检查'

            sym_labels, sym_label_id, labels, label_id, sym_tone_labels, sym_tone_label_id = get_yinjie_label(cmd)

            labels = labels.strip()
            print("获取声韵母id: ")
            print("%s --> %s --> %s"%(cmd, sym_labels, sym_label_id))
            print("获取声韵母+音调id: ")
            print("%s --> %s --> %s"%(cmd, sym_tone_labels, sym_tone_label_id))
            print("获取音节id: ")
            print("%s --> %s --> %s\n"%(cmd, labels, label_id))
            #cmd_id_f.write(cmd + ' --> ' + '[' + ' '.join(label_id)  + ']' + '\n')

    print ("Done!")

