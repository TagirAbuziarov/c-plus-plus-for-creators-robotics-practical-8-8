#pragma once

#include <QWidget>
#include <QKeyEvent>
#include <QImage>
#include <QPoint>
#include <QList>
#include <QRegion>

#define SNAKE_IMAGE_FOLDER "data"
#define SNAKE_APPLE_IMAGE_FILE_NAME "apple.png"
#define SNAKE_BODY_IMAGE_FILE_NAME "body.png"
#define SNAKE_HEAD_IMAGE_FILE_NAME "head.png"
#define SNAKE_GAME_OVER_IMAGE_FILE_NAME "game_over.png"

#define SNAKE_CELL_SIZE 20
#define SNAKE_H_CELLS 30
#define SNAKE_V_CELLS 30
#define SNAKE_WINDOW_WIDTH (SNAKE_H_CELLS*SNAKE_CELL_SIZE)
#define SNAKE_WINDOW_HEIGHT (SNAKE_V_CELLS*SNAKE_CELL_SIZE)

#define SNAKE_INIT_LENGTH 3
#define SNAKE_INIT_HEAD_CELL_X 14  // 0-based
#define SNAKE_INIT_HEAD_CELL_Y 14  // 0-based

#define SNAKE_UPDATE_PERIOD_MS 900

class Snake : public QWidget
{
public:
    Snake(QWidget* parent = 0);

protected:

    /// @brief Загрузка изображений змейки и яблока
    void loadImages();

    /** 
     * @brief Создает змейку, случайным образом располагает яблоко на игровом
     * поле и запускает игру 
     **/
    void initGame();

    /**
     * @brief Расположение змейки. В секции define указаны координаты ячейки
     * в которой будет расположена голова змейки. Тело располагается ниже
     * вертикально
     **/
    void locateSnake();
    
    /// @brief Случайное расположение яблока на игровом поле
    void locateApple();

    /// @brief Обработка нажатий кнопок клавиатуры. Наследовано от QWidget
    void keyPressEvent(QKeyEvent *event) override;

    /// @brief Обработка события таймера. Наследовано от QWidget
    void timerEvent(QTimerEvent *event) override;

    /**
     * @brief Обработка игрового цикла: перемещение тела змейки, проверка 
     * и обработка коллизий, вызов перерисовки игрового поля
     **/
    void updateGame();

    /// @brief Перемещает тело змейки на одну ячейку
    void moveSnake();

    /// @brief Проверка коллизий игровых объектов друг с другом
    bool checkCollision();

    /// @brief Возвращает true, если змейка врезалась в стену
    bool isSnakeCollideWall();

    /// @brief Возвращает true, если змейка врезалась в собственное тело
    bool isSnakeSelfCollide();

    /// @brief Возвращает true, если змейка касается яблока головой
    bool isAppleCollideSnakeHead();

    /// @brief Возвращает true, если змейка касается яблока телом
    bool isAppleCollideSnakeBody();

    /// @brief Возвращает true, если змейка выросла и не помещается на поле
    bool isSnakeTooBig();

    /// @brief Останавливает игру
    void onEndGame();

    /// @brief Перерисовка окна. Наследовано от QWidget
    void paintEvent(QPaintEvent *event) override;

    
    /// @brief Изображение яблока
    QImage apple_image_;

    /// @brief Изображение головы змейки
    QImage head_image_;

    /// @brief Изображение тела змейки
    QImage body_image_;

    /// @brief Изображение надписи конца игры
    QImage game_over_image_;

    /// @brief Список координат тела змейки, пиксели
    QList<QPoint> snake_bodypart_coordinates_;

    /// @brief Список форм тела змейки для проверки на коллизии
    QList<QRegion> snake_bodypart_collisions_;

    /// @brief Координаты яблока, пиксели
    QPoint apple_coordinates_;

    /// @brief Форма яблока для проверки на коллизии
    QRegion apple_collision_;

    /// @brief Направление движения змейки на текущем игровом цикле
    double direction_;

    /// @brief Направление движения змейки на предыдущем игровом цикле
    double direction_prev_;

    /** 
     * @brief Флаг съеденного яблока. Если true, тело удлиннится в следующем
     * цикле
     **/
    bool is_apple_eaten_;

    /** 
     * @brief Флаг конца игры. Если true, выводится надпись "game over", новые
     * игровые циклы не происходят
     **/
    bool is_game_over_;

    /// @brief id таймера времени циклов
    int timerId_;

    /// @brief Размер ячеек игрового поля
    static const QSize cell_size_;
};