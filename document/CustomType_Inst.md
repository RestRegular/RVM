### CustomType (�Զ�������)
- #### ����
    CustomType ��ʾһ���Զ�����������ͣ����԰�������Ԫ�أ�

  - �����ֶ� (tpFields) - �������ͱ�����ֶ�
  - ʵ���ֶ� (instFields) - ����ʵ�����ֶ�
  - �����ֶ� (methodFields) - ��Ϊʵ�����������ͷ���

- #### ��Ҫ����
  - �̳л��ƣ�ÿ�� CustomType ������һ�������� (parentType)

  - �ֶι���
  
    - ֻ��ͨ�� CustomType ����ֶ� 
    - �����ֶ�ֻ��ͨ�� CustomType ���� 
    - ʵ���ֶ�ֻ��ͨ�� CustomInst ���� 
    - ���ͼ�飺�ṩ checkBelongTo() ����������͹�ϵ

- #### ��Ҫ����
  - `addTpField()/addInstField()` - �������/ʵ���ֶ�
  - `setTpField()` - ���������ֶ�ֵ
  - `getTpField()` - ��ȡ�����ֶ�ֵ
  - `hasField()/hasInstField()` - ����ֶδ�����

### CustomInst (�Զ�������ʵ��)
- #### ����
    CustomInst ��ʾ CustomType ��ʵ�������������ص㣺

  - ����������ֶ�
  - ֻ������ʵ���ֶκ͵���ʵ������
  - ���Է���ʵ���ֶκ������ֶ�

- #### ��Ҫ����
  - �ֶδ洢��ʹ��Ƕ��ӳ��洢ʵ���ֶ� (instFields)
  - ���͹�����ÿ��ʵ������һ�� CustomType (customType)
  - ����ת����֧�� derivedToChildType() ����ת��Ϊ������

- #### ��Ҫ����
  - `getField()` - ��ȡ�ֶ�ֵ����ָ���ض����ͣ�
  - `setField()` - ����ʵ���ֶ�ֵ
  - `hasField()` - ����ֶδ�����

### ʹ�ù�ϵ
1. ���ȶ��� CustomType�����������ֶ�
2. Ȼ�󴴽� CustomInst ʵ��
3. ͨ�� CustomInst ����ʵ���ֶ�
4. ͨ�������� CustomType ���������ֶ�