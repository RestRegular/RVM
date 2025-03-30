## RA���Թ淶

#### [@RestRegular](https://github.com/RestRegular) 2025/3/22 13:27:03

### һ������

#### 1.1 ������Ŀ��

RA���ԣ�Rio Assembly Language����һ�������ָ��ʽ������ԣ�רΪ RVM��Rio Virtual Machine���������ơ�����ҪĿ����Ϊ RVM
�ṩһ�ָ�Ч��������������ĵͼ����ԣ�����ʵ�� RVM ��ָ���RA���Ե���������Դ�ڴ�ͳ������ԣ������﷨�ṹ����������Ͻ������Ż����Ը��õ���Ӧ
RVM �����л�����

ͨ�� RA���ԣ������߿���ֱ�ӱ�д�ײ�ָ����� RVM ���ڴ�ռ��ջ�������Ӷ�ʵ�ָ����ܵ�Ӧ�ó��򿪷���ͬʱ��RA����ҲΪ�߼����Ա������ṩ���м��ʾ�㣬ʹ���������Կ���ͨ����������
RA���룬���������� RVM �ϡ�

#### 1.2 ���ԭ��

RA���Ե������ѭ���º���ԭ��

- **�����** ���﷨�ṹ�����򵥣�����ѧϰ�ɱ���ͬʱ���ڱ������ͽ�������ʵ�֡�
- **��Ч��** ��ָ�������� RVM ��Ӳ������㣬ȷ��ִ��Ч����󻯡�
- **����չ��** ��֧��δ����ָ�����չ�������������ܶ����ƻ����д���ļ����ԡ�
- **������** �������ǵͼ����ԣ������ʱ�ṩ��һ���ĳ����������綯̬����ϵͳ�ȣ��������������顣

#### 1.3 Ӧ�ó���

RA�������������³�����

- **�ײ㿪��** ��ֱ�ӱ�д RVM ��ԭ��ָ�����ʵ�ֲ���ϵͳ�ںˡ����������������Ҫ�����ܵ������
- **���������** ����Ϊ�߼����ԣ��� Rio��Python��C �ȣ���������Ŀ�����ԣ����ɸ�Ч�� RVM ��ִ�д��롣
- **��ѧ���о�** ��������ӽ�Ӳ�������ԣ�RA���Էǳ��ʺ����ڼ������ϵ�ṹ�ͱ���ԭ��Ľ�ѧ���о���
- **Ƕ��ʽϵͳ** ������Դ���޵Ļ����У�RA���Կ�����Ϊһ��������������ѡ��

#### 1.4 ���������ԵĶԱ�

RA���Ե���ƽ���˴�ͳ������Ե��ص㣬����֮��������²�ͬ��

- **��ǿ���﷨�ṹ** ��RA����������ͳһ���﷨�ṹ������ָ��ı�д����⡣
- **�����ר��** ��RA����רΪ RVM ��ƣ������ָ��� RVM �ļܹ��߶�ƥ�䣬����ͳ�������ͨ���������Ӳ����
- **��ƽ̨֧��** ������ RVM ��һ���������RA�����������п�ƽ̨���ԣ����迼�ǵײ�Ӳ�����졣

#### 1.5 �ĵ��ṹ

���ĵ�����ϸ���� RA���Ե������ʹ�÷�������Ҫ�����������ݣ�

1. [x] **�ʷ��ṹ** ������ RA���ԵĴʷ����򣬰����ؼ��֡���ʶ����ע�ͺͷָ�����
2. [x] **�﷨�ṹ** ������ RA���ԵĻ����﷨���򣬰���ָ���ʽ���������͡�
3. [x] **�������** ������ÿ��ָ��ľ�����Ϊ������ RVM �ϵ�ִ�й��̡�
4. [ ] **��׼��**���г� RA�������õı�׼�⺯��������;��
5. [ ] **����ʱ**������ RA������ RVM �ϵ����л��ƣ������ڴ����ջ�����ȡ�
6. [ ] **������** ������ RA����������ʱ���������Ĵ��󣬲��ṩ��Ӧ�Ĵ�������
7. [ ] **������**������ RA������ RVM ֮��ļ����ԣ�����ָ����������͵ȡ�
8. [ ] **ʾ������** ��ͨ��ʵ�ʰ���չʾ RA���Ե�ʹ�÷�����

### �����ʷ��ṹ

#### 2.1 �ַ���

RA����֧�������ַ�����

- **Ӣ����ĸ** ��`a-z`��`A-Z`
- **����** ��`0-9`
- **�����ַ�** ���������������������ַ���
    - ע�ͷ���`;`������ע�ͷ���
    - ���ӷ���`~`
    - �ָ�����`:`��`,`
    - �ַ�����ʾ����`"`
    - �������ո��Ʊ�������з��ȿհ��ַ�

�������������������ַ��������򽫱���Ϊ�Ƿ��ַ���

#### 2.2 ��ʶ��

��ʶ������������������ǩ��������ʵ�塣RA���Եı�ʶ���������£�

- ��������ĸ��A-Z �� a-z�����»��ߣ�_����ͷ��
- �����ַ���������ĸ�����ֻ��»��ߡ�
- ��ʶ�����ִ�Сд��
- ����û����ȷ���ƣ������鲻���� 32 ���ַ�����߿ɶ��ԡ�

**ʾ����**

```ra
valid_identifier    ; ���»�������ȫСд��ĸ�ı�ʶ��ͨ��������������
_valid              ; ���»��߿�ͷ�����ı�ʶ��ͨ�������������ر���
variable123         ; ��ʶ�����԰����������ֲ�ͬʵ��
FunctionIdentifier  ; ��������ĸ��д�ı�ʶ��ͨ�������������� 
VARIABLE            ; ȫ��д�ı�ʶ��ͨ��������������
_123                ; ���Ƽ���ʵ�庬��ı�ʶ��
_                   ; ��ȫ�»��߿�ͷ�ı�ʶ��ͨ���������������Եı���
```

**�Ƿ�ʾ����**

```ra
123variable  ; ���󣺲��������ֿ�ͷ
@temp        ; ���󣺰����Ƿ��ַ�
```

#### 2.3 ָ�

RA���Ե�ָ���RIS���Ǳ����֣�ָ�� RVM �ܹ�ֱ��ִ�еĲ���������������ʶ����

RIS�ɷ�Ϊ���¼��ࣺ

- **����ָ��** �����ض�ִ�в�����ָ�
- **��־ָ��** ������Ϊ��־λ������ָ�
- **��ʽָ��** ����ִ�в�����ָ�ͨ�����ڴ���ռλ��

������ RA���Ե�ָ����

<table>
    <tr>
        <th rowspan="2" colspan="2"> ���</th>
        <th rowspan="2"> ָ��</th>
        <th rowspan="2"> ����</th>
        <th colspan="2"> ����</th>
        <th rowspan="2"> ����ʾ��</th>
        <th rowspan="2"> ��ע</th>
    </tr>
    <tr>
        <th> ����</th>
        <th> ����</th>
    </tr>
    <tr>
        <td rowspan="30"> ����ָ��</td>
        <td rowspan="4"> �ڴ����</td>
        <td style="text-align: center;"><code>ALLOT</code></td>
        <td> Ϊ���ݷ���ʵ���ڴ�ռ�</td>
        <td style="text-align: center;"> [0, n]</td>
        <td><code>ALLOT</code>ָ���������������ʵ���ڴ�ռ䲢�����Բ�������</td>
        <td>
      <pre>
ALLOT:a,b,c
ALLOT:;
</pre>
        </td>
        <td> ��RA�����У�ʹ�ñ���ǰ����Ҫ��Ϊ������ڴ�ռ䣬������Ϊ���Զ������ڴ�ռ䣬��������ֶ������ڴ�</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>DELETE</code></td>
        <td> �ͷ�ʵ���ڴ�ռ�</td>
        <td style="text-align: center;"> [0, n]</td>
        <td><code>DELETE</code>ָ��������ͷŲ�����Ӧ��ʵ���ڴ�ռ䣬�ͷź󲻿ɱ�����</td>
        <td>
      <pre>
ALLOT:a,b,c
DELETE:a,b,c</pre>
        </td>
        <td> RA������Ȼ֧���ֶ��ͷ��ڴ棨��ʹ�� <code>DELETE</code> ָ�����RVM���Զ������ڴ棬��˲������ֶ��ͷ��ڴ�ռ䡣
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>PUT</code></td>
        <td> ������д��ʵ���ڴ�ռ�</td>
        <td style="text-align: center;"> 2</td>
        <td> ��1Ϊ�����������ݣ���2Ϊ��д�������ʵ��</td>
        <td>
      <pre>
ALLOT:a,b
PUT:10,a
PUT:a,b</pre>
        </td>
        <td style="text-align: center;" rowspan="2"> ֵ��ע����ǣ�<code>PUT</code> ָ���ǽ�ԭʼ���ݽ����ƶ�д�룬�����Ǵ��������ݸ��������贴����������ʹ�� <code>COPY</code> ָ�<code>PUT</code>�ڲ�����ֵ���ͣ�Numeric��������ʱ���ȼ���<code>COPY</code></td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>COPY</code></td>
        <td> ��ԭʼ���ݴ�����������������д��Ŀ��ʵ���ڴ�ռ�</td>
        <td style="text-align: center;"> 2</td>
        <td> ��1Ϊ�����������ݣ���2Ϊ��д�������ʵ��</td>
        <td>
      <pre>
ALLOT:a, b
PUT:10, a
COPY:a, b</pre>
        </td>
    </tr>
    <tr>
        <td rowspan="4"> �������</td>
        <td style="text-align: center;"><code>ADD</code></td>
        <td> �ӷ�����</td>
        <td style="text-align: center;"> 3</td>
        <td> ��1����2Ϊ�����������ݣ���3Ϊ��д�������ʵ��</td>
        <td>
      <pre>
ALLOT:a
ADD:1,1,a</pre>
        </td>
        <td rowspan="4"> ֻ��������ֵ���ͣ�Numeric��</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>OPP</code></td>
        <td> ȡ������</td>
        <td style="text-align: center;"> 2</td>
        <td> ��1Ϊ�����������ݣ���2Ϊ��д�������ʵ��</td>
        <td>
      <pre>
ALLOT:a
OPP:1,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>MUL</code></td>
        <td> �˷�����</td>
        <td style="text-align: center;"> 3</td>
        <td> ��1����2Ϊ�����������ݣ���3Ϊ��д�������ʵ��</td>
        <td>
      <pre>
ALLOT:a
MUL:2,5,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>DIV</code></td>
        <td> ��������</td>
        <td style="text-align: center;"> 3</td>
        <td> ��1����2Ϊ�����������ݣ���3Ϊ��д�������ʵ��</td>
        <td>
      <pre>
ALLOT:a
DIV:10,2,a</pre>
        </td>
    </tr>
    <tr>
        <td> ��Χ����</td>
        <td style="text-align: center;"><code>END</code></td>
        <td> ��Ǿֲ������λ��</td>
        <td style="text-align: center;"> [0, 1]</td>
        <td> �����������ڱ�ǽ����ľֲ������ƣ���ʡ�ԣ���Ϊ����ߴ���ɶ��ԣ�������ʡ��</td>
        <td>
      <pre>
FUNC:Func
  ... ; ������
END:Func</pre>
        </td>
        <td><code>END</code>ָ�����ʶ�ֲ���Ľ���λ�ã�ͬʱ RVM
            ������ִ�д�ָ��ʱ�Զ������ͷžֲ����з�����ڴ�ռ䣬���<code>END</code>���ڲ���ָ��
        </td>
    </tr>
    <tr>
        <td rowspan="5"> ��������</td>
        <td style="text-align: center;"><code>FUNC</code></td>
        <td> �޷���ֵ��������</td>
        <td style="text-align: center;"> [1, n]</td>
        <td> ��1Ϊ����������2����nΪ��������</td>
        <td>
      <pre>
FUNC:NoneReturnFunc,a,b,c
  ... ; ������
END:NoneReturnFunc</pre>
        </td>
        <td style="text-align: center;" rowspan="2"> /</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>FUNI</code></td>
        <td> �з���ֵ��������</td>
        <td style="text-align: center;"> [2, n]</td>
        <td> ��1Ϊ����������2����nΪ��������</td>
        <td>
      <pre>
FUNI:HasReturnFunc,a,b,c
  ... ; ������
END:HasReturnFunc</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>CALL</code></td>
        <td> �޷���ֵ��������</td>
        <td style="text-align: center;"> [1, m]</td>
        <td> ��1Ϊ����������2����mΪ����������m-1��<code>FUNC</code>ָ���ĺ��������������</td>
        <td>
      <pre>
FUNC:NoneReturnFunc,a,b,c
  ... ; ������
END:NoneReturnFunc
CALL:NoneReturnFunc,1,2,3</pre>
        </td>
        <td> ֻ������<code>FUNC</code>ָ���ĺ���</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>IVOK</code></td>
        <td> �з���ֵ��������</td>
        <td style="text-align: center;"> [2, m]</td>
        <td> ��1Ϊ����������mΪ����ֵ����ʵ�壬��2����m-1Ϊ����������m-2��<code>FUNI</code>ָ���ĺ��������������</td>
        <td>
      <pre>
FUNI:HasReturnFunc,a,b,c
  ... ; ������
END:HasReturnFunc
ALLOT:return_value
IVOK:HasReturnFunc,1,2,3,return_value</pre>
        </td>
        <td> ֻ������<code>FUNI</code>ָ���ĺ���</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RET</code></td>
        <td> ��������ֵ����</td>
        <td style="text-align: center;"> 1</td>
        <td> ����Ϊ�����ķ���ֵ</td>
        <td>
      <pre>
FUNI:HasReturnFunc,a,b,c
  ... ; ������
  RET:10
END:HasReturnFunc</pre>
        </td>
        <td> ֻ������<code>FUNI</code>ָ���ĺ���</td>
    </tr>
    <tr>
        <td> �Ƚϲ���</td>
        <td style="text-align: center;"><code>CMP</code></td>
        <td> �Ƚ����ݹ�ϵ����</td>
        <td style="text-align: center;"> 3</td>
        <td> ��1����2Ϊ���Ƚϵ����ݣ���3Ϊ��д�������ʵ��</td>
        <td>
      <pre>
ALLOT:a,b,cmp
CMP:a,b,cmp</pre>
        </td>
        <td style="text-align: center;"> /</td>
    </tr>
    <tr>
        <td rowspan="6"> ���Ʋ���</td>
        <td style="text-align: center;"><code>REPEAT</code></td>
        <td> ѭ��ִ�д���ָ��</td>
        <td style="text-align: center;"> 1</td>
        <td> ����Ϊѭ��ִ�еĴ���</td>
        <td>
      <pre>
REPEAT:10
  ... ; ѭ����
END:REPEAT</pre>
        </td>
        <td style="text-align: center;" rowspan="2"> /</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>UNTIL</code></td>
        <td> ѭ������ָ��ظ�ִ��ֱ����������</td>
        <td style="text-align: center;"> 2</td>
        <td> ��1Ϊѭ��������ͨ������<code>CMP</code>��ȡ�ıȽ������͵����ݣ���2Ϊ��ϵ��־����</td>
        <td>
      <pre>
ALLOT:a,b,cmp
CMP:a,b,cmp
UNTIL:cmp,RE
  ... ; ѭ����
END:UNTIL</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>EXIT</code></td>
        <td> �˳��ֲ���ָ��</td>
        <td style="text-align: center;"> [0, 1]</td>
        <td> �����������ڱ���˳��ľֲ������ƣ���ʡ�ԣ���Ϊ����ߴ���ɶ��ԣ�������ʡ��</td>
        <td>
      <pre>
FUNC:Func
  ... ; ������
  EXIT:Func
END:Func</pre>
        </td>
        <td><code>EXIT</code>ָ������ھֲ����е�����λ��ʹ�ã��� RVM
            ִ�д�ָ��ʱ�Ὣִ��������һ��Ŀ��ָ������Ϊ�þֲ����<code>END</code>ָ��
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SET</code></td>
        <td> ���ñ�ǩָ��</td>
        <td style="text-align: center;"> 1</td>
        <td> ����Ϊ��ǩ��</td>
        <td>
      <pre>
SET:label</pre>
        </td>
        <td>
            ��Ҫע�����<code>SET</code>ָ�����õı�ǩ��������ʵ�壬������ñ�ǩ����Ҫִ��<code>ALLOT</code>�����ڴ�ռ�Ĳ�����ʵ���ϱ�ǩ�Ǿֲ�������ԣ������ڴ�ռ��ɾֲ������
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>JMP</code></td>
        <td> ��������תָ��</td>
        <td style="text-align: center;"> 1</td>
        <td> ����Ϊ��ǩ��</td>
        <td>
      <pre>
JMP:label2
SET:label1
  JMP:label3
SET:label2
  JMP:label1
SET:label3</pre>
        </td>
        <td> ��Ϊ��ǩ�Ǿֲ�������ԣ���� RVM ������ʱ����ֲ���ʱ������ִ�б�ǩ�����ò��������<code>JMP</code>ָ������ھֲ��������λ����ת����ǰ�ֲ���ı�ǩ��Ŀǰ�޷�������ת��ToDo��Ҳ��δ����ʵ�ֿ�����ת��
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>JR</code></td>
        <td> ������תָ��</td>
        <td style="text-align: center;"> 3</td>
        <td> ��1Ϊ���ݱȽ��飬ͨ����<code>CMP</code>��ȡ�Ƚ������͵����ݣ���2Ϊ��ϵ��־��������3Ϊ��ת��ǩ��</td>
        <td>
      <pre>
ALLOT:a,b,cmp
CMP:a,b,cmp
JR:cmp,RE,label1
JMP:label2
SET:label1
SET:label2</pre>
        </td>
        <td>
            <code>JR</code>ָ����<code>JMP</code>ָ�����ƣ�ֻ����ת����ǰ�ֲ���ı�ǩ��Ŀǰ�޷�������ת��ToDo��Ҳ��δ����ʵ�ֿ�����ת��
        </td>
    </tr>
    <tr>
        <td rowspan="2"> IO����</td>
        <td style="text-align: center;"><code>SOUT</code></td>
        <td> ������ݵ���׼���</td>
        <td style="text-align: center;"> [1, n]</td>
        <td> ��1�����ģʽ��־ָ�����Ϊ�����������</td>
        <td>
      <pre>
SOUT:s-l,a,b,c</pre>
        </td>
        <td>
            <a href="#io">���ģʽ��־ָ��</a>����<code>s-l</code>�������������<code>s-m</code>�����������
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SIN</code></td>
        <td> �������ݵ���׼����</td>
        <td style="text-align: center;"> [1, n]</td>
        <td> ��1������ģʽ��־ָ�����Ϊ�����������</td>
        <td>
      <pre>
SIN:s-l,a,b,c</pre>
        </td>
        <td>
            <a href="#io">����ģʽ��־ָ��</a>����<code>s-l</code>���������룩��<code>s-m</code>���������룩
        </td>
    </tr>
    <tr>
        <td rowspan="2"> ���ò���</td>
        <td style="text-align: center;"><code>QOT</code></td>
        <td> �������ݲ���</td>
        <td style="text-align: center;"> 2</td>
        <td> ��1Ϊ�����õ����ݣ���2Ϊ��д�������ʵ�壬ע�⣺��������������������ʵ��</td>
        <td>
      <pre>
ALLOT:a,b
QOT:a,b</pre>
        </td>
        <td>
            <code>QOT</code>ָ��ֻ��������������ʵ�壬�����Զ���Ŀ������ʵ������͸���Ϊ�������ͣ���������Ŀ���ID�洢��Ŀ������ʵ����
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>QOT_VAL</code></td>
        <td> �޸��������ݲ���</td>
        <td style="text-align: center;"> 2</td>
        <td> ��1Ϊ���޸ĵ����ݣ���2Ϊ��д�������ʵ�壬ע�⣺��2�����ͱ���Ϊ��������</td>
        <td>
      <pre>
ALLOT:a,b
QOT:a,b
QOT_VAL:10,b</pre>
        </td>
        <td>
            <code>QOT_VAL</code>ָ��ֻ�������޸���������ʵ�壬���ὫĿ�����ݴ洢����������ʵ�����õ�����ʵ����
        </td>
    </tr>
    <tr>
        <td rowspan="2"> ���Ͳ���</td>
        <td style="text-align: center;"><code>TP_SET</code></td>
        <td> �޸�����ʵ�����Ͳ���</td>
        <td style="text-align: center;"> 2</td>
        <td> ��1Ϊ���ͱ�־ָ���2Ϊ���޸ĵ�����ʵ��</td>
        <td>
      <pre>
ALLOT:a,b
TP_SET:tp-int,a</pre>
        </td>
        <td>
            <a href="#customType">���ͱ�־ָ��</a>����<code>tp-int</code>�����ͣ���<code>tp-float</code>�������ͣ���<code>tp-str</code>���ַ�������<code>tp-bool</code>�������ͣ���<code>tp-char</code>���ַ����ͣ���<code>tp-null</code>�������ͣ���<code>ALLOT</code>ָ����������ʵ������Ĭ��Ϊ<code>tp-null</code>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>TP_GET</code></td>
        <td> ��ȡ����ʵ�����Ͳ���</td>
        <td style="text-align: center;"> 2</td>
        <td> ��1Ϊ����ȡ������ʵ�壬��2Ϊ�������ݴ�д�������ʵ��</td>
        <td>
      <pre>
ALLOT:a,b
TP_GET:a,b</pre>
        </td>
        <td>
            <code>TP_GET</code>ָ����Ի�ȡ���ݺ�ʵ������ͣ����ὫĿ������ʵ������ʹ洢��Ŀ������ʵ����
        </td>
    </tr>
    <tr>
        <td rowspan="4"> ���������</td>
        <td style="text-align: center;"><code>SP_GET</code></td>
        <td> ��ȡ������ID����</td>
        <td style="text-align: center;"> 1</td>
        <td> ����Ϊ���������ݴ�д�������ʵ��</td>
        <td>
      <pre>
ALLOT:a
SP_GET:a</pre>
        </td>
        <td>
            <code>SP_GET</code>ָ����Ի�ȡ������ID�����Ὣ��ǰ�������ID�洢��Ŀ������ʵ����
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SP_SET</code></td>
        <td> �޸ĵ�ǰ���������</td>
        <td style="text-align: center;"> 1</td>
        <td> ����Ϊ�洢������ID������ʵ��</td>
        <td>
       <pre>
ALLOT:a
SP_SET:a</pre>
        </td>
        <td>
            <code>SP_SET</code>ָ������޸ĵ�ǰ���������Ὣ��ǰ���������޸�Ϊָ��ID��������
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SP_NEW</code></td>
        <td> �½��ֲ����������</td>
        <td style="text-align: center;"> 1</td>
        <td> ����Ϊ����ʵ�壬���ڴ洢�½��������ID</td>
        <td>
       <pre>
ALLOT:a
SP_NEW:a</pre>
        </td>
        <td>
            <code>SP_NEW</code>ָ������½��ֲ����������Ὣ�½��������ID�洢��Ŀ������ʵ����
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SP_DEL</code></td>
        <td> ɾ���ֲ����������</td>
        <td style="text-align: center;"> 1</td>
        <td> ����Ϊ�洢������ID������ʵ��</td>
        <td>
       <pre>
ALLOT:a
SP_GET:a
SP_DEL:a</pre>
        </td>
        <td>
            <code>SP_DEL</code>ָ�����ɾ���ֲ����������ὫĿ��ID��������ɾ�������Ƽ�ʹ�ô�ָ����������ɴ����ֲ���ָ����ľֲ����磺<code>FUNC</code>��<code>REPEAT</code>�ȣ�����Ϊ RVM ���Զ���������ָ����ľֲ���
        </td>
    </tr>
    <tr>
        <td rowspan="18"> ��־ָ��</td>
        <td rowspan="4"> IO��־<a id="io"></a></td>
        <td style="text-align: center;"><code>s-l</code></td>
        <td> �������ģʽ</td>
        <td style="text-align: center;" rowspan="4"> 0</td>
        <td style="text-align: center;" rowspan="4"> /</td>
        <td>
      <pre>
SOUT:s-l,...</pre>
        </td>
        <td>
            <code>s-l</code>��ʾ�������ģʽ�����������ʱ����������ݶ���ֱ����ǰ������ݽ���ƴ�����
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>s-m</code></td>
        <td> �������ģʽ</td>
        <td>
        <pre>
SOUT:s-m,...</pre>
        </td>
        <td>
            <code>s-m</code> ��ʾ�������ģʽ�����������ʱ����������ݻ��Ի��з���ǰ������ݽ���ƴ�����
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>s-f</code></td>
        <td> ˢ����������־</td>
        <td>
        <pre>
SOUT:s-l,...,s-f,...</pre>
        </td>
        <td>
            <code>s-f</code> ����������ˢ���������ı�־����RVM�У��������ȱ�����������棬������ﵽһ�������������նˡ�ʹ�� <code>s-f</code> ����ǿ�ƽ���ǰ�����е�������������������صȴ��������ء�
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>s-n</code></td>
        <td> ���б�־</td>
        <td>
        <pre>
SOUT:s-l,...,s-n,...</pre>
        </td>
        <td>
            <code>s-n</code>��ʾ���б�־�����������ʱ���ڱ�־λ��������з�
        </td>
    </tr>
    <tr>
        <td rowspan="6"> ���ͱ�־<a id="customType"></a></td>
        <td style="text-align: center;"><code>tp-int</code></td>
        <td> ���ͱ�־</td>
        <td style="text-align: center;" rowspan="6"> 0</td>
        <td style="text-align: center;" rowspan="6"> /</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-int,a</pre>
        </td>
        <td rowspan="6">
            ���ͱ�־��ʾ���Ӧ���������ͣ����ڲ��洢��ֵ�����Ӧ���͵�ID�ַ��������������������������ַ������ͣ�����ʹ��<code>TP_GET</code>�鿴������
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-float</code></td>
        <td> �����ͱ�־</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-float,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-str</code></td>
        <td> �ַ�����־</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-str,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-bool</code></td>
        <td> ������־</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-bool,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-null</code></td>
        <td> ��ֵ��־</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-null,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-char</code></td>
        <td> �ַ���־</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-char,a</pre>
        </td>
    </tr>
    <tr>
        <td rowspan="8"> ��ϵ��־<a id="relationship"></a> </td>
        <td style="text-align: center;"><code>RL</code></td>
        <td> ���ڹ�ϵ��־ </td>
        <td style="text-align: center;" rowspan="8"> 0</td>
        <td style="text-align: center;" rowspan="8"> /</td>
        <td>
        <pre>
ALLOT:a
TP_GET:RL,a</pre>
        </td>
        <td style="text-align: center;" rowspan="8"> /</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RLE</code><a id="relation-RLE"></a></td>
        <td> ���ڵ��ڹ�ϵ��־ </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RLE,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RNE</code><a id="relation-RNE"></a></td>
        <td> �����ڹ�ϵ��־ </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RNE,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RE</code><a id="relation-RE"></a></td>
        <td> ���ڹ�ϵ��־ </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RE,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RSE</code><a id="relation-RSE"></a></td>
        <td> С�ڵ��ڹ�ϵ��־ </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RSE,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RS</code><a id="relation-RS"></a></td>
        <td> С�ڹ�ϵ��־ </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RS,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RT</code><a id="relation-RT"></a></td>
        <td> ���־ </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RT,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RF</code><a id="relation-RF"></a></td>
        <td> �ٱ�־ </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RF,a</pre>
        </td>
    </tr>
    <tr>
        <td rowspan="2"> ��ʽָ��</td>
        <td rowspan="2"> </td>
        <td style="text-align: center;"><code>PASS</code></td>
        <td> ռλ��ָ��</td>
        <td style="text-align: center;"> [0, n]</td>
        <td style="text-align: center;"> /</td>
        <td>
        <pre>
PASS:;</pre>
        </td>
        <td>
            ռλ��ָ�����ռλ����ָ��ִ��ʱ��RVM ����Դ�ָ�ֱ��ִ�к���ָ��
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>UNKNOWN</code></td>
        <td> δָ֪��</td>
        <td style="text-align: center;"> [0, n]</td>
        <td style="text-align: center;"> /</td>
        <td>
        <pre>
UNKNOWN:;</pre>
        </td>
        <td>
            ���� RVM ��ʶ��������δָ֪��
        </td>
    </tr>
</table>

#### 2.4 ����
RA����֧���������͵ĳ�����

- **��ֵ���ͣ�Numeric��**
  - **��������** �� �� `123`, `-456`
  - **���㳣��** �� �� `3.14`, `-0.001`
  - **��������** �� ֻ������ֵ��`RT` �� `RF`��Ҳ�����ڱ�ʶ��ϵ��
- **�ɵ������ͣ�Iterable��**
  - **�ַ�������** �� ʹ��˫������ס���� `"Hello, World!"`��
  ֧��ת���ַ����� `\n`�����У���`\t`���Ʊ������`\"`��˫���ţ���
- **�����ͣ�Null��**
  - **��ֵ����** �� ֻ��һ��ֵ��`null`��

**ʾ����**
```Ra
ALLOT:a,b,c,d,e
PUT:10,a              ; ��������
PUT:3.14,b            ; ���㳣��
PUT:"Hello, World!",c ; �ַ�������
PUT:true,d            ; ��������
PUT:null,e            ; ��ֵ����
```

#### 2.5 ��ϵ
RA����֧���������͵Ĺ�ϵ��

- **���ڹ�ϵ**��`RL`
- **���ڵ��ڹ�ϵ**��`RLE`
- **�����ڹ�ϵ**��`RNE`
- **���ڹ�ϵ**��`RE`
- **С�ڵ��ڹ�ϵ**��`RSE`
- **С�ڹ�ϵ**��`RS`
- **���ϵ**��`RT`
- **�ٹ�ϵ**��`RF`

��ϸ������ [��ϵ��־](#relationship)��

#### 2.6 ע��
RA����֧�ֵ���ע�ͺͶ���ע�ͣ�
- **����ע��** ��

  �Էֺţ�;����ͷ��ֱ����β���������ݾ�����Ϊע�͡�

  **ʾ����**
  ```Ra
  ; ����һ������ע��
  ALLOT:a ; Ϊ����a�����ڴ�ռ�
  PUT:10,a ; ��10�洢������ʵ��a
  ```
- **����ע��** ��
  
  ����ע�Ϳ���ʹ��`~`���`;`��ɡ�

  **ʾ����**
  ```Ra
  ; ����һ������ע��
  ~ ���Ƕ���ע�͵ĵڶ���
  ~ ���Ƕ���ע�͵ĵ�����
  ALLOT:a ; Ϊ����a�����ڴ�ռ�
  ```
  
**ע��**_`~`�����ӷ���RVM �ڽ�������ʱ�Ὣ��ͷ����`~`��һ��ƴ�ӵ�������һ�У��������չʾ�Ķ���ע��ʵ����Ҳֻ�ǵ���ע�͡�_

#### 2.7 �ָ���
RA����ʹ�����·ָ�������֯����ṹ��

- **�ֺţ�;��** �����ڽ���һ��ָ�������ע�͡�
- **���ţ�,��** �����ڷָ��������������
- **ð�ţ�:��** �����ڷָ�ָ��Ͳ�����

**ʾ����**
```Ra
ALLOT:a,b,c,d,e;    �ֺſ������ڽ���һ��ָ�������ע��
PUT:10,a;           �������ڷָ������������
SOUT:s-l,a,b,c,d,e; ð�����ڷָ�ָ��Ͳ���
```

#### 2.8 �հ��ַ�
�հ��ַ����ո��Ʊ�������з����� RA�����б����ԡ�

**ʾ����**
```Ra
ALLOT:a,b,c; û�пհ��ַ�
ALLOT : d, e, f; �пհ��ַ������ǽ���ʱ�����
```

### �����﷨�ṹ

RA���Ե��﷨�ṹ����ͳһ���������Ƿ���mnemonic����ʽ��ʾָ������ʽ���£�

```Ra
[RI] : <[Arg], [Arg], ...> (;)
```

#### 3.1 ָ���ʽ����
�����Ǹ��﷨�ṹ�и����ֵ���ϸ˵����
- [**RI**] ��
  - ��ʾָ��Ĳ����루Opcode������ָ��ĺ��Ĳ��֡�
  - ���� RA�����е�һ����Чָ�����ƣ����� `ALLOT`��`PUT`��`SOUT`��... �ȡ�
  - ָ��ͨ��������ĸ���»�����ɣ������ִ�Сд��
  - ������ϴʷ��ṹ�ж����ָ���

- **:** ��
  - �ָ�������������б�ķ��š�
  - ���﷨�ṹ�й̶��ķָ�����������ȷָ����������Ĺ�ϵ��

- <[**Arg**], [**Arg**], ...> ��
  - �����б�����ָ������Ĳ�������
  - ����֮���ö��ţ�`,`���ָ���
  - �������������ݡ�����ʵ�塢�ؼ��ֵȣ���������ȡ����ָ���Ҫ��
  - ������������������ָ��Ķ��������
  
  **ʾ����**
  ```
  ALLOT:a,b; �����������ֱ�������ʵ������a��b��
  ADD:10,20,a; �����������ֱ���Ŀ��洢����ʵ����������ݡ�
  ```

- (`;`) ��
  - ��ѡ�ķֺ�`;`�����ڽ���һ��ָ�
  - ���ʡ�Էֺţ���Ĭ���Ի��з���Ϊָ��Ľ�����־�� 
  - �ֺź�������ע�ͣ�����˵��ָ��Ĺ��ܻ���;��

#### 3.2 ʾ������
������һЩ���� RA�����﷨�ṹ��ʾ�����룺

1. **���ݴ���ָ��** ��

   ```Ra
   ALLOT:a,b
   PUT:10,a
   PUT:a,b
   ```

2. **��������ָ��** ��

   ```Ra
   ALLOT:a,b,c,d
   ; ����10+20������洢������ʵ��a��
   ADD:10,20,a
   ; ����30-a������洢������ʵ��b��
   OPP:a,a; ȡ����ʵ�ּ�������
   ADD:30,a,b
   ; ����2*5������洢������ʵ��c��
   MUL:2,5,c
   ; ����c/2������洢������ʵ��d��
   DIV:c,2,d
   ```

3. **���Ʋ���ָ��** ��

    ```Ra
    JMP:label3
    SET:label1
        JMP:label2
    SET:label2
        JMP:label_end
    SET:label3
        JMP:label1
    SET:label_end
    ```

4. **ϵͳ����ָ��** ��

    ```Ra
    PASS:; �ղ�������ִ���κβ���
    ```

#### 3.3 ��������
RA����֧�ֶ������͵Ĳ���������������¼��֣�

- **��ʱ����** ��

  ����`10`��`12.5`��`"Hello world!"`����ֱ����ֵ���ֵĳ�Ϊ**��ʱ����**��RVM ���ڴ����Զ���������Щ��ʱ���ݣ������ڴ�ķ�������ա����ǵ��ض���ֻ���ڵ�ǰָ��ִ���ڼ���Ч��ִ�н�����ᱻ�Զ��ͷš�

  - **��������**
    - **����**
    - **������**
    - **����ֵ**
    
    ������� [��ֵ����](#24-����)��
  - **�ַ�������**
    
    ������� [�ַ�������](#24-����)��

- **����ʵ��** ��

  **����ʵ��**�� RA �����еĺ��ĸ�����ڹ���Ͳ������ݡ�����ʵ����ͨ�� `ALLOT` ָ����ģ�������һ������ʵ�壬RVM ��Ϊ�������ʵ�����һ���ڴ�ռ䣬��ͨ��IDӳ����ƽ�ָ���ı�ʶ���������ڴ�ռ�ID����ӳ�䣬��ʵ����ʱ���ݵĴ洢�ͷ��ʡ�����ʱ���ݱ���������ʵ���RVM ��������ָ���ִ�����ڽ������Զ��ͷ�������ݣ�ֱ�������˳�����������ʵ��ľֲ���ʱ��RVM ���Զ��ͷ��������ʵ����ռ�õ��ڴ�ռ��Ա����ڴ�й©��
  
  - **��ʶ��**
    
    ͨ�� `ALLOT` ָ���������ʵ��ı�ʶ����������ʵ������ơ�
  - **�ؼ���**

    RVM �Դ�������ʵ��ؼ��֣�������
    - `SN` �������ֶ��洢��������ֵ������ʵ�塣
    - `SR` �������Զ��洢��������ֵ������ʵ�塣
    - `SE` �������Զ��洢������Ϣ������ʵ�塣
    - `SS` �������ֶ��洢�ֲ�����Ϣ������ʵ�塣

#### 3.5 �﷨����
Ϊ��ȷ�� RA���Ե��﷨һ���ԣ����������¹���

1. **ָ��˳��** ��
    
    ָ�˳��ִ�У�������������ת��ָ��� `JMP`��`CALL`��`IVOK`�ȣ���
    
    **����** ��
    ```Ra
    ALLOT:a
    ADD:1,2,a       ; �� ALLOT ָ��ִ��֮��ADD ָ��Żᱻִ�С�
    ```
2. **��������ƥ��** ��
   
   ÿ��ָ��Ĳ���������������䶨��Ҫ��
   
   **����** ��
   
   ```Ra
   ALLOT:a
   PUT:10,a        ; ��ȷ��PUT ָ����Ҫ��������
   PUT:10          ; ����ȱ�ٵڶ�������
   ```

3. **��������ƥ��** ��

   �������ͱ�����ָ���Ҫ��һ�¡�

   **����** ��

   ```Ra
   ALLOT:a,b,c     ; ��ȷ��ALLOT ָ����Ҫ������ʶ������
   ALLOT:1,2,3     ; ����ALLOT��֧����ʱ���͵Ĳ���
   ```

4. **�ֺŵ�ʹ��** �� 
   
   �ֺ��ǿ�ѡ�ģ��������ע����ʹ�ã�������ʡ�ԡ�
   **ʾ��** ��
   ```Ra
   ALLOT:a,b,c;   ; �Ϸ�
   ALLOT:a,b,c    ; ���Ƽ�
   ```

### �ġ���׼��

### �塢����ʱ

### ����������

### �ߡ�������������

### �ˡ�ʾ������