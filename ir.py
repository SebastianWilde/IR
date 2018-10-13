import heapq
import psycopg2.extras
import math
from collections import OrderedDict

def get_vu(list):
    magnitud = 0
    for ele in list:
        if ele:
            magnitud += ele**2
    magnitud = math.sqrt(magnitud)
    v_u = []
    for ele in list:
        if ele:
            v_u.append(ele/magnitud)
        else:
            v_u.append(0)
    return v_u


conn = psycopg2.connect("host=localhost dbname=CorpusProcesado user=postgres password=")
cur = conn.cursor(cursor_factory = psycopg2.extras.DictCursor)


lema=input()

query_nombre="""
select id from lemas where lema='%s'
"""%(lema)
# print("query_nombres")
# print(query_nombre)

DEC2FLOAT = psycopg2.extensions.new_type(
    psycopg2.extensions.DECIMAL.values,
    'DEC2FLOAT',
    lambda value, curs: float(value) if value is not None else None)
psycopg2.extensions.register_type(DEC2FLOAT)

cur.execute(query_nombre)

res= cur.fetchone()

lema_id = res['id']

query_dimensiones ="""
select id2 from nllinker where id1=%d limit 1598
"""%(int(lema_id))

cur.execute(query_dimensiones)
res= cur.fetchall()
dimensiones = ','.join([str(x['id2'])  for x in  res])

#Calcular vector unitario
magnitud = 0
list_dim = [int(dim) for dim in dimensiones.split(',')]
for dim in list_dim:
    magnitud+=dim**2
magnitud = math.sqrt(magnitud)
# print(magnitud)
v_unitario = []
for dim in list_dim:
    v_unitario.append(dim/magnitud)

# vector_calculo = ["idnulo"]+[(x['v_unitario'])  for x in  res]
vector_calculo = ["idnulo"] + v_unitario
# print(vector_calculo)
###

cabecera = '"id1" decimal,' + ','.join([ '"' +str(x['id2']) +'" decimal' for x in res])

query_vectores="""
SELECT * FROM crosstab(
  $$ SELECT id1, id2, freq FROM nllinker where id2 in(%s) order by id1 $$,
  $$ SELECT id2 from nllinker where id1=%d limit 1598 $$
) AS (
  %s
)

"""%(dimensiones,int(lema_id),cabecera)

# print("query_vectores")
# print(query_vectores)

cur.execute(query_vectores)
vectores= cur.fetchall()
# print(vectores[1])
# print(vector_calculo)
dict_res=dict()
print("obtendiendo producto punto...")
for row in vectores:
    new_row= get_vu(row[1:])
    producto_punto=0
    id1= int(row[0])
    for i in range(1,len(row)):
        a=float(vector_calculo[i])
        b=(new_row[i-1])
		#3print "multiplicando ",a," ",b
        producto_punto+=a*b if b else 0
	#print producto_punto
    dict_res[id1]=producto_punto

sorted_x = sorted(dict_res.items(), key=lambda x: x[1])
# print(sorted_x[-10:])
final_res = []
for key,value in sorted_x[:10]:
	final_res.append(key)
	print(key,value)

for key,value in sorted_x[-10:]:
	final_res.append(key)
	print(key,value)
# print(final_res)
query_res_final ="""
select lema from lemas where id in (%s)
"""%(','.join([str(x) for x in final_res]))

# print("query_final ")
# print(query_res_final)

cur.execute(query_res_final)
res_final = cur.fetchall()

print(res_final)