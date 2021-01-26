/* Copyright (C) 
 * 2021 - francisco.rodriguez at ingenieria dot unam dot edu, (fjrg76)
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */


/*
 * query_3.cpp
 *
 * Este ejemplo extrae los datos de la base de datos y los guarda en una lista
 * para su posterior procesamiento.
 *
 * Tuve que usar C++ para std::vector ya que no quise liarme con memoria
 * dinámica. En C eventualmente tendríamos que usar realloc() y crear un tipo
 * abstracto que emule a std::vector.
 *
 * NOTA: Su aplicación la pueden escribir únicamente en C y luego compilarla con
 * C++. Nada más no olviden que éste último es mucho más estricto:
 *
 * $ g++ -Wall -osalida.out sqlite3.o -lpthread -ldl query_3.cpp
 *
 * Antes de ejecutar el programa recuerden tener una base de datos con la tabla
 * alumnos y algunos registros (pueden usar al programa insert_1.c).
 */

#include <vector>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sqlite3.h" 


// La tabla alumnos tiene más columnas (campos) pero, en este ejemplo, sólo nos
// interesan el nombre y el promedio:

typedef struct
{
   char nombre[80];
   double promedio;
} Data;


/**
 * @brief Callback para las consultas (query)
 *
 * @param data Campo de uso para el usuario. El usuario la usa como quiera.
 * @param argc Número de columnas encontradas
 * @param argv Valor (como texto) de cada columna
 * @param col_name Nombre (en la base de datos) de cada columna
 *
 * @return 0: ok, !0: algún error
 */
int get_datos( void* data, int argc, char** argv, char** col_name )
{
//      printf( "%s / %s\n", argv[ 0 ], argv[ 1 ] ); // para depuración

   auto lista = (std::vector<Data>*) data;

   Data datos;

   // La búsqueda que hicimos incluye ÚNICAMENTE a los campos nombre y promedio:

   strncpy( datos.nombre, argv[ 0 ], 80 );
   datos.promedio = atof( argv[ 1 ] == NULL ? "0.0" : argv[ 1 ] );

   lista->push_back( datos );

   return 0;
}

//----------------------------------------------------------------------
//  Driver program
//----------------------------------------------------------------------
int main(int argc, char* argv[]) 
{
   sqlite3 *db;

   int rc = sqlite3_open("test_db.sqlite3", &db);

   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else { fprintf(stderr, "Opened database successfully\n"); }


   const char* sql = "SELECT nombre, promedio FROM alumnos;";
   // ¡Mucho cuidado con la búsqueda! Los campos y tipos de éstos deben coincidir 
   // con aquellos que hayan especificado en la estructura Data

   // NO SON BÚSQUEDAS GENERALES NI GENÉRICAS. SQLite es una base de datos
   // embebida y las búsquedas, en su mayoría, serán especializadas.

   char* err_msg = NULL;

   std::vector<Data> lista;
   // a este vector (arreglo dinámico) lo usamos básicamente como estructura de
   // datos Queue (cola), ya que sólo sirve como puente entre SQLite y nuestra
   // aplicación

   rc = sqlite3_exec( 
            db, 
            sql,
            get_datos,      // la callback y
            (void*) &lista, // los campos deben coincidir
            &err_msg );

   if( rc != SQLITE_OK ){
      fprintf( stderr, "SQL error: %s\n", err_msg );
      sqlite3_free( err_msg );
   } else{ 
      size_t regs = lista.size();

      printf( "Encontré %ld registros:\n", regs );

      for( size_t i = 0; i < regs; ++i ){

         // si estuvieran usando tablas hash, árboles o grafos, aquí los
         // llenarían (en lugar de imprimir). Aunque también podrían pasar una
         // de éstas directamente a la callback en lugar del vector.

         printf( 
            "Nombre:   %s\n"
            "Promedio: %lf\n\n",
            lista[ i ].nombre,
            lista[ i ].promedio );

      }
      fprintf( stdout, "\nHecho!\n" ); 
   }

   sqlite3_close(db);
}
