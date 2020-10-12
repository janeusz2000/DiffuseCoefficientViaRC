#include "objects.h"
#include <sstream>
namespace objects
{
#pragma region OBJECT

    void Object::setOrigin(const core::Vec3 & or)
    {
        _origin = or ;
    }
    core::Vec3 Object::getOrigin() const
    {
        return _origin;
    }

#pragma endregion
#pragma region SPHERE

    core::Vec3 Sphere::normal(const core::Vec3 &surface_point) const
    {
        return (surface_point - getOrigin()).normalize();
    }

    Sphere::Sphere(const core::Vec3 & or, const double &rad)
    {
        this->setOrigin(or);
        _radius = rad;
    }

    bool Sphere::hitObject(const core::Ray &ray, const double &freq, core::RayHitData &hitData)
    {
        core::Vec3 rVec3 = ray.getOrigin() - this->getOrigin();

        // this calculates variables that are neccesary to calculate times at which ray hits the object SphereWall.
        double beta = 2 * rVec3.scalarProduct(ray.getDirection());
        double gamma = rVec3.scalarProduct(rVec3) - _radius * _radius;
        double discriminant = beta * beta - 4 * gamma;
        // ==================================================

        // If object is at opposite direction than ray is:
        if (discriminant < 0)
        {
            return false;
        }

        double temp = std::sqrt(discriminant);
        double time1 = (-beta - temp) / 2;
        double time2 = (-beta + temp) / 2;

        // if both objects are at opposite direction then ray is:
        if (time1 < 0)
        {
            return false;
        }
        else if (time1 < 0 && time2 < 0)
        {
            return false;
        }
        else
        {
            double time = std::min(time1, time2);
            core::Vec3 collision = ray.at(time);
            hitData = core::RayHitData(time, normal(collision), ray, freq);
            return true;
        }
    }

    std::ostream &operator<<(std::ostream &os, const Sphere &sp)
    {
        return os << "Sphere origin: " << sp.getOrigin() << ", radius: " << sp.getRadius() << " [m]";
    }

    // GETTERS AND SETTERS
    void Sphere::setRadius(const double &rad)
    {
        _radius = rad;
    }
    double Sphere::getRadius() const
    {
        return _radius;
    }

#pragma endregion
#pragma region SPHEREWALL

    SphereWall::SphereWall()
    {
        this->setOrigin(core::Vec3(0, 0, 0));
        this->setRadius(constants::kSimulationRadius);
    }

    std::ostream &operator<<(std::ostream &os, const SphereWall &sp)
    {
        return os << "SphereWall origin: " << sp.getOrigin() << ", radius: " << sp.getRadius() << " [m]";
    }

#pragma endregion
#pragma region ENERGY_COLLECTOR

    // OPERATORS
    EnergyCollector &EnergyCollector::operator=(const EnergyCollector &other)
    {
        if (other == *this)
        {
            return *this;
        }

        this->setOrigin(other.getOrigin());
        this->setRadius(other.getRadius());
        _energy = other.getEnergy();

        return *this;
    }

    std::ostream &operator<<(std::ostream &os, const EnergyCollector &collector)
    {
        return os << "Energy Collector ID: " << collector.getID() << ", Energy collected: " << collector.getEnergy();
    }

    EnergyCollector operator+(const EnergyCollector &left, const EnergyCollector &right)
    {
        if (left.getOrigin() != right.getOrigin())
        {
            std::stringstream ss;
            ss << "Energy Collectors doesn't have the same origin positions. Positions are left: " << left.getOrigin() << ", right: " << right.getOrigin();
            throw exception::differentPositions(ss.str());
        }
        else
        {
            return EnergyCollector(left.getOrigin(), left.getEnergy() + right.getEnergy());
            // TODO: This may cause issues, when you want to keep collectors in a list of unique_ptrs.
            // Since this one creates a new object and it seems that energy collectors are just data objects.
            // My guess would be that you want to mutate collectors?
        }
    }

    bool objects::operator==(const EnergyCollector &left, const EnergyCollector &right)
    {
        return (left.getID() == right.getID() && left.getOrigin() == right.getOrigin() && left.getRadius() == right.getRadius() && left.getEnergy() == right.getEnergy());
    }

    // METHODS

    double EnergyCollector::distanceAt(const core::Vec3 &positionHit) const
    {
        return (getOrigin() - positionHit).magnitude();
    }

    void EnergyCollector::collectEnergy(const core::RayHitData &hitdata)
    {
        // TODO: Energy distribution between many collectors
        _energy += hitdata.energy;
    }

    // GETTERS AND SETTERS
    void EnergyCollector::setEnergy(const double &en)
    {
        _energy = en;
    }
    double EnergyCollector::getEnergy() const
    {
        return _energy;
    }
    void EnergyCollector::addEnergy(const double &en)
    {
        _energy += en;
    }
    void EnergyCollector::setID(const int &id)
    {
        _id = id;
    }
    int EnergyCollector::getID() const
    {
        return _id;
    }

#pragma endregion
#pragma region TRIANGLEOBJ

    // CONSTRUCTORS

    TriangleObj::TriangleObj() : TriangleObj(core::Vec3(1, 0, 0), core::Vec3(0, 1, 0), core::Vec3(0, 0, 1))
    {
        this->refreshAttributes();
    }
    TriangleObj::TriangleObj(const core::Vec3 &point1, const core::Vec3 &point2, const core::Vec3 &point3) : _point1(point1), _point2(point2), _point3(point3)
    {
        this->arePointsValid();
        this->refreshAttributes();
    }

    TriangleObj::TriangleObj(const TriangleObj &other)
    {
        this->setX(other.getX());
        this->setY(other.getY());
        this->setZ(other.getZ());
        this->refreshAttributes();
    }

    // OPERATORS

    TriangleObj &TriangleObj::operator=(const TriangleObj &other)
    {
        this->setX(other.getX());
        this->setY(other.getY());
        this->setZ(other.getZ());

        this->refreshAttributes();

        return *this;
    }

    bool operator==(const TriangleObj &left, const TriangleObj &right)
    {
        std::vector<core::Vec3> vertexVec({left.getX(), left.getY(), left.getZ()});
        std::vector<core::Vec3> refVec({right.getX(), right.getY(), right.getZ()});

        for (size_t ind = 0; ind < vertexVec.size(); ++ind)
        {
            if (std::find(vertexVec.begin(), vertexVec.end(), refVec.at(ind)) == vertexVec.end())
            {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const TriangleObj &left, const TriangleObj &right)
    {
        return (!(left == right));
    }

    std::ostream &operator<<(std::ostream &os, const TriangleObj &object)
    {
        return os << "Triangle Object, vertex: " << object.getX() << ", " << object.getY() << ", " << object.getZ();
    }

    // METHODS

    core::Vec3 TriangleObj::normal(const core::Vec3 &surface_point) const
    {
        return _normal;
    }

    bool TriangleObj::hitObject(const core::Ray &ray, const double &freq, core::RayHitData &hitData)
    {
        // if ray direction is parpedicular to normal, there is no hit. It can be translated into
        // checking if scalarProduct of the ray.direction and normal is close or equal to zero.
        double parpCoeff = ray.getDirection().scalarProduct(_normal);
        if (std::abs(parpCoeff) <= constants::kAccuracy)
        {
            return false;
        }

        // Following code calculates time at which ray is hitting surface where triangle is positioned
        double time = (-1 * (ray.getOrigin() - _point3)).scalarProduct(_normal) / parpCoeff;

        // Following code is making sure that ray doesn't hit the same object.
        if (time < constants::kHitAccuracy)
        {
            return false;
        }

        core::Vec3 surfaceHit = ray.at(time);

        if (this->doesHit(surfaceHit))
        {
            hitData = core::RayHitData(time, _normal, ray, freq);
            return true;
        }
        return false;
    }

    bool TriangleObj::doesHit(const core::Vec3 &point) const
    {
        core::Vec3 vecA = _point1 - point;
        core::Vec3 vecB = _point2 - point;
        core::Vec3 vecC = _point3 - point;

        double alpha = vecB.crossProduct(vecC).magnitude() / 2; //  Area of the triangle made with point and w triangle points.
        double beta = vecC.crossProduct(vecA).magnitude() / 2;
        double gamma = vecA.crossProduct(vecB).magnitude() / 2;

        // TODO: Why do you multiply by 0.99? Consider making it a constant and using it here.
        return (((alpha + beta + gamma) > _area + constants::kHitAccuracy * 0.99) ? false : true);
    }

    double TriangleObj::area() const
    {
        return _area;
    }

    void TriangleObj::refreshAttributes()
    {
        this->recalculateArea();
        this->recalculateNormal();
        this->setOrigin((_point1 + _point2 + _point3) / 3);
    }

    // PRIVATE METHODS

    // TODO: Why do you have all these separate methods recalculating some small parts,
    // while you could just move all the code into refreshAttributes() and cache the results.
    void TriangleObj::recalculateArea()
    {
        core::Vec3 vecA = _point1 - _point2;
        core::Vec3 vecB = _point1 - _point3;
        _area = vecA.crossProduct(vecB).magnitude() / 2;
    }

    void TriangleObj::recalculateNormal()
    {
        core::Vec3 vecA = _point1 - _point2;
        core::Vec3 vecB = _point1 - _point3;
        core::Vec3 perpendicular = vecA.crossProduct(vecB);
        _normal = perpendicular.normalize();
    }

    bool TriangleObj::arePointsValid()
    {
        if (_point1 == _point2 || _point1 == _point3 || _point2 == _point3)
        {
            std::stringstream ss;
            ss << "TriangleObj arguments error. You cannot have duplications of the same point in constructor 1: " << _point1 << ", 2: " << _point2 << ", 3: " << _point3;
            throw std::invalid_argument(ss.str());
        }

        core::Vec3 alpha = _point1 - _point2;
        core::Vec3 beta = _point1 - _point3;

        if (alpha.crossProduct(beta) == core::Vec3(0, 0, 0))
        {
            throw std::invalid_argument("Triangle object couldn't be constructed. You cannot have all points of the object linedup straight");
        }
        return true;
    }

    // GETTERS AND SETTERS
    core::Vec3 TriangleObj::getX() const
    {
        return _point1;
    }
    void TriangleObj::setX(const core::Vec3 &point)
    {
        this->_point1 = point;
    }

    core::Vec3 TriangleObj::getY() const
    {
        return _point2;
    }
    void TriangleObj::setY(const core::Vec3 &point)
    {
        this->_point2 = point;
    }

    core::Vec3 TriangleObj::getZ() const
    {
        return _point3;
    }
    void TriangleObj::setZ(const core::Vec3 &point)
    {
        this->_point3 = point;
    }

#pragma endregion
} // namespace objects