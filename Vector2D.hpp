#pragma once
#include <iostream>

class Vector2D {
    public:  
        float x;
        float y;

        Vector2D();
        Vector2D(float x, float y);

        Vector2D& Add(const Vector2D& vec);
        Vector2D& Subtract(const Vector2D& vec);
        Vector2D& Multiply(const Vector2D& vec);
        Vector2D& Divide(const Vector2D& vec);

        friend Vector2D& operator+(Vector2D& v1, const Vector2D& v2);
        friend Vector2D& operator-(Vector2D& v1, const Vector2D& v2);
        friend Vector2D& operator*(Vector2D& v1, const Vector2D& v2);
        friend Vector2D& operator/(Vector2D& v1, const Vector2D& v2);

        Vector2D& operator+=(const Vector2D& vec);
        Vector2D& operator-=(const Vector2D& vec);
        Vector2D& operator*=(const Vector2D& vec);
        Vector2D& operator/=(const Vector2D& vec);

        Vector2D& operator*(const int& i);
        Vector2D& Zero();

        friend std::ostream& operator<<(std::ostream& stream, const Vector2D& vec);

        Vector2D operator/(float scalar) const { return Vector2D(x / scalar, y / scalar); }
        float magnitude() const;
        Vector2D normalize() const;

        bool operator==(const Vector2D& other) const {
            return x == other.x && y == other.y;
        }
        
        bool operator!=(const Vector2D& other) const {
            return !(*this == other);
        }
        
        bool operator<(const Vector2D& other) const {
            return (x < other.x) || (x == other.x && y < other.y);
        }
};