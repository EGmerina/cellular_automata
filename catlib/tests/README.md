# Testing

Для библиотеки catlib была была реализована система тестирования c помощью gitlab CI/CD YAML.

При выгрузке коммитов в gitlab последний коммит будет автоматически тестироваться на сервере. 
Это займет несколько минут, по завершению тестирования при наличии ошибки на связанную с аккаунтом почту будет выслано уведомление об ошибке.

Разрабатываемые модели, находящиеся в папке dev, не должны добавляться в систему тестирования.

Готовые же модели, хранящиеся в папке models, должны быть добавлены в систему тестирования.

Для локального запуска тестирования запустите файл /tests/build.sh.

### Compiling
	./build.sh 

### Добавление новых моделей
Чтобы добавить новую модель в систему тестирования измените 4 следующих файла:


### 1. CMakeLists.txt
В файле /tests/CMakeLists.txt добавить if для нового MODEL_NUMBER. MODEL_NUMBER - номер модели, указываемый при запуске cmake, для компиляции только тех файлов, что нужны для этой модели.
#### 
	if(${MODEL_NUMBER} STREQUAL "YOUR_MODEL_NUMBER")
	endif()
###
Внутри if для каждого компилируемого файла вашей модели (обычно prep, sim и post) 
	Добавьте add_executable с названием скомпилированного файла, например (test_your_model_prep),
	и путем до компилируемого файла, например (../your_model/your_model_prep.c).

### 2. build.sh 
В файле /tests/build.sh добавьте:

Комментарий с названием вашей модели.
	
rm -Rf * для очистки директории.

cmake с вашей топологией, режимом использованием mpi/omp и "MODEL_NUMBER" добавленный вами в CMakeLists.txt
	во всех возможных конфигурациях.
####
	cmake .. -DTOPOLOGY=yourTopology -DOPER_MODE=(ASYNC or SYNC) -DUSE_OMP=(TRUE or FALSE) -DUSE_MPI=(TRUE or FALSE) -DMODEL_NUMBER=YOUR_MODEL_NUMBER

make для компиляции файлов
	И запуск ваших скомпилированных файлов, например (./test_your_model_prep).

### 3. .gitlab-ci.yml
В файле .gitlab-ci.yml, который отвечает за запуск тестирования на сервере, добавьте всё то же, что вы добавили в файле build.sh, перед каждой строкой добавляя "-".

### 4. README.md
В конце файла /tests/README.md добавьте название вашей модели и в каких конфигурациях cmake она запускается.

## Тестируемые модели и их ключи компиляции

#### phase_sep
	cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=SYNC -DMODEL_NUMBER=001
	cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=SYNC -DUSE_MPI=TRUE -DMODEL_NUMBER=001
	cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=SYNC -DUSE_OMP=TRUE -DMODEL_NUMBER=001


#### domino_sync
	cmake .. -DTOPOLOGY=SQUARE24 -DOPER_MODE=SYNC -DMODEL_NUMBER=005
	cmake .. -DTOPOLOGY=SQUARE24 -DOPER_MODE=SYNC -DUSE_MPI=TRUE -DMODEL_NUMBER=005
	cmake .. -DTOPOLOGY=SQUARE24 -DOPER_MODE=SYNC -DUSE_OMP=TRUE -DMODEL_NUMBER=005


#### diff_bool_asynch
	cmake .. -DTOPOLOGY=SQUARE4 -DOPER_MODE=ASYNC -DMODEL_NUMBER=002
	cmake .. -DTOPOLOGY=SQUARE4 -DOPER_MODE=ASYNC -DUSE_OMP=TRUE -DMODEL_NUMBER=002


#### save_mass_phase_sep_async
	cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=ASYNC -DMODEL_NUMBER=003
	cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=ASYNC -DUSE_OMP=TRUE -DMODEL_NUMBER=003


#### FHP-MP
	cmake .. -DTOPOLOGY=HEXAGON -DOPER_MODE=SYNC -DMODEL_NUMBER=004
	cmake .. -DTOPOLOGY=HEXAGON -DOPER_MODE=SYNC -DUSE_OMP=True -DMODEL_NUMBER=004