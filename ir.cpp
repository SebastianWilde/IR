//g++ -std=c++11 -o ir ir.cpp -L/usr/lib/x86_64-linux-gnu -lpq
#include <iostream>
#include </usr/include/postgresql/libpq-fe.h>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <math.h>
PGconn *cnn = NULL;
PGresult *result = NULL;
using namespace std;
char *host = "localhost";
char *port = "5432";
char *dataBase = "CorpusProcesado";
char *user = "postgres";
char *passwd = "";

vector<float> get_vu(vector<float> vec){
    vector<float> vu;
    float magnitud = 0;
    for (int i=0;i<vec.size();i++){
        magnitud += pow(vec[i],2);
    }
    magnitud = sqrt(magnitud);
    for (int i=0;i<vec.size();i++){
        vu.push_back(vec[i]/magnitud);
    }
    return vu;

}

int main(int argc, char const *argv[])
{
    int i;
    cnn = PQsetdbLogin(host,port,NULL,NULL,dataBase,user,passwd);
    if (PQstatus(cnn) != CONNECTION_BAD) {
        cout << "Estamos conectados a PostgreSQL!" << endl;
        string palabra;
        cout<<"Ingresa la palabra:";
        cin>>palabra;
        string query = "select id from lemas where lema='"+palabra+"'";
        result = PQexec(cnn, query.c_str());
        if (result != NULL) {
            string lema_id = PQgetvalue(result,0,0);
            PQclear(result);
            string query_dimensiones = "select id2 from nllinker where id1="+lema_id+"limit 1598";
            result = PQexec(cnn, query_dimensiones.c_str());
            string dimensiones="";
            vector<float> vector_dimensiones;
            string cabecera_dimensiones="";
            for (int i=0; i<PQntuples(result);i++){
                string val = PQgetvalue(result,i,0);
                vector_dimensiones.push_back(strtof(val.c_str(),0));
                dimensiones+=val;
                dimensiones+=",";
                cabecera_dimensiones += "decimal,\""+val+"\" ";
            }
            PQclear(result);
            dimensiones = dimensiones.substr(0,dimensiones.size()-1);
            vector<float> vector_dimensiones_u = get_vu(vector_dimensiones);

            // cout<<endl<<dimensiones<<endl;

            string cabecera ="\"id1\""+cabecera_dimensiones+"decimal";

            string query_vectores = "SELECT * FROM crosstab($$ SELECT id1, id2, freq FROM nllinker where id2 in("+
            dimensiones+") order by id1 $$,$$ SELECT id2 from nllinker where id1="+lema_id+" limit 1598 $$ ) AS ("+
            cabecera+")";
            // cout<<"Query vectores"<<endl<<query_vectores;

            result = PQexec(cnn, query_vectores.c_str());
            map <string,float> resultados;
            for (int i=0; i<PQntuples(result);i++){
                string id = PQgetvalue(result,i,0);
                vector<float> new_vec;
                float magnitud_vec=0;
                float producto_punto = 0;
                for (int j=1;j<PQnfields(result);j++){
                    if(PQgetisnull(result,i,j) == 0){
                        float val = strtof(PQgetvalue(result,i,j),0);
                        magnitud_vec +=  pow(val,2);
                        new_vec.push_back(val);
                    }
                    else{
                        new_vec.push_back(0);
                    }
                }
                magnitud_vec = sqrt(magnitud_vec);
                for (int k = 0; k<vector_dimensiones_u.size();k++){
                    producto_punto += vector_dimensiones_u[k]*new_vec[k]/magnitud_vec;
                }
                resultados[id] = producto_punto;
            }
            PQclear(result);
            // Declaring the type of Predicate that accepts 2 pairs and return a bool
	        typedef std::function<bool(std::pair<std::string, float>, std::pair<std::string, float>)> Comparator;
            // Defining a lambda function to compare two pairs. It will compare two pairs using second field
	        Comparator compFunctor =
                [](std::pair<std::string, float> elem1 ,std::pair<std::string, float> elem2)
                {
                    return elem1.second > elem2.second;
                };

            // Declaring a set that will store the pairs using above comparision logic
	        std::set<std::pair<std::string, float>, Comparator> setOfWords(
			resultados.begin(), resultados.end(), compFunctor);

            //Sorting
            vector<float> ranking;
            string ids_respuestas = "";
            int number_respuestas = 0;
            for (std::pair<std::string, float> element : setOfWords){
                ranking.push_back(element.second);
                ids_respuestas += element.first + ",";
                if (number_respuestas > 10)
                    break;
                number_respuestas+=1;
		        // std::cout << element.first << " :: " << element.second << std::endl;
            }

            ids_respuestas = ids_respuestas.substr(0,ids_respuestas.size()-1);
            //Resultado final
            string query_final = "select lema from lemas where id in ("+ids_respuestas+")";
            result = PQexec(cnn, query_final.c_str());
            for(int t = 0;t<PQntuples(result);t++){
                cout<<PQgetvalue(result,t,0)<<" "<<ranking[t]<<endl;
            }
            PQclear(result);
        }

    } 
    else {
        cout << "Error de conexion" << endl;
        return 0;
    }

    PQfinish(cnn);
    return 0;
}
/*https://thispointer.com/how-to-sort-a-map-by-value-in-c/*/
/*
https://logbinario.wordpress.com/2010/01/09/conexion-c-con-postgresql-mediante-libpq/
int tuplas = PQntuples(result);
            int campos = PQnfields(result);
            cout << "No. Filas:" << tuplas << endl;
            cout << "No. Campos:" << campos << endl;

            cout << "Los nombres de los campos son:" << endl;

            for (i=0; i<campos; i++) {
                cout << PQfname(result,i) << " | ";
            }

            cout << endl << "Contenido de la tabla" << endl;

            for (i=0; i<tuplas; i++) {
                for (int j=0; j<campos; j++) {
                    cout << PQgetvalue(result,i,j) << " | ";
                }
                cout << endl;
            }
*/