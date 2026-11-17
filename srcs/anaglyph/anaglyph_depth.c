/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   anaglyph_depth.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/03 00:00:00 by user             #+#    #+#             */
/*   Updated: 2025/01/03 00:00:00 by user            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <minirt.h>
#include <float.h>

void	populate_z_buffer(t_minirt *s)
{
	int		y;
	int		x;
	float	pixel_center[3];
	float	ray_direction[3];
	int		idx;
	t_ray	ray;

	ft_bzero(s->scene.cam.z_buffer, W * H * sizeof(float));
	y = -1;
	while (++y < H)
	{
		x = -1;
		while (++x < W)
		{
			pixel_center[0] = s->scene.vp.pixel00l[0]
				+ (x * s->scene.vp.deltah[0]) + (y * s->scene.vp.deltav[0]);
			pixel_center[1] = s->scene.vp.pixel00l[1]
				+ (x * s->scene.vp.deltah[1]) + (y * s->scene.vp.deltav[1]);
			pixel_center[2] = s->scene.vp.pixel00l[2]
				+ (x * s->scene.vp.deltah[2]) + (y * s->scene.vp.deltav[2]);
			vec3_subf(ray_direction, pixel_center, s->scene.cam.o);
			ray = get_ray(s->scene.cam.o, ray_direction);
			idx = y * W + x;
			s->scene.cam.z_buffer[idx] = ray_distance(s->scene.objects, &ray);
		}
	}
}

void	visualize_z_buffer(t_minirt *s)
{
	int		i;
	float	min_depth;
	float	max_depth;
	float	normalized;
	int		grayscale;
	unsigned int	*pixel_ptr;
	int		hit_count;
	int		miss_count;

	ft_bzero(s->scene.cam.z_buffer_viz, (W + 32) * H * 4);
	min_depth = INFINITY;
	max_depth = 0.0f;
	hit_count = 0;
	miss_count = 0;
	i = -1;
	while (++i < W * H)
	{
		if (s->scene.cam.z_buffer[i] > 0.0f)
		{
			hit_count++;
			if (s->scene.cam.z_buffer[i] < min_depth)
				min_depth = s->scene.cam.z_buffer[i];
			if (s->scene.cam.z_buffer[i] > max_depth)
				max_depth = s->scene.cam.z_buffer[i];
		}
		else
			miss_count++;
	}
	printf("Z-buffer: hits=%d, misses=%d, range=[%.6f, %.6f]\n",
		hit_count, miss_count, min_depth, max_depth);
	if (min_depth == INFINITY || max_depth <= min_depth)
	{
		printf("Z-buffer visualization: no valid depth data\n");
		return ;
	}
	min_depth = logf(min_depth);
	max_depth = logf(max_depth);
	pixel_ptr = (unsigned int *)s->scene.cam.z_buffer_viz;
	i = -1;
	while (++i < W * H)
	{
		if (s->scene.cam.z_buffer[i] > 0.0f)
		{
			normalized = (logf(s->scene.cam.z_buffer[i]) - min_depth)
				/ (max_depth - min_depth);
			normalized = powf(normalized, 0.5f);
			normalized = 1.0f - normalized;
			grayscale = (int)(normalized * 255.0f);
			grayscale = clamp(grayscale, 0, 255);
			pixel_ptr[i] = (grayscale << 16)
				| (grayscale << 8) | grayscale;
		}
		else
			pixel_ptr[i] = 0x000000FF;
	}
	printf("Z-buffer visualization complete\n");
}

void	populate_z_buffer_red(t_minirt *s)
{
	int		y;
	int		x;
	float	pixel_center[3];
	float	ray_direction[3];
	int		idx;
	t_ray	ray;
	float	z_min;
	float	z_max;

	ft_bzero(s->scene.cam.z_buffer_red, W * H * sizeof(float));
	z_min = FLT_MAX;
	z_max = -FLT_MAX;
	y = -1;
	while (++y < H)
	{
		x = -1;
		while (++x < W)
		{
			pixel_center[0] = s->scene.vp.pixel00l[0]
				+ (x * s->scene.vp.deltah[0]) + (y * s->scene.vp.deltav[0]);
			pixel_center[1] = s->scene.vp.pixel00l[1]
				+ (x * s->scene.vp.deltah[1]) + (y * s->scene.vp.deltav[1]);
			pixel_center[2] = s->scene.vp.pixel00l[2]
				+ (x * s->scene.vp.deltah[2]) + (y * s->scene.vp.deltav[2]);
			vec3_subf(ray_direction, pixel_center, s->scene.cam.o);
			ray = get_ray(s->scene.cam.o, ray_direction);
			idx = y * W + x;
			s->scene.cam.z_buffer_red[idx] = ray_distance(s->scene.objects, &ray);
			if (s->scene.cam.z_buffer_red[idx] < z_min)
				z_min = s->scene.cam.z_buffer_red[idx];
			if (s->scene.cam.z_buffer_red[idx] > z_max)
				z_max = s->scene.cam.z_buffer_red[idx];
		}
	}
	printf("Z_buffer_red populated: min=%.2f max=%.2f\n", z_min, z_max);
}

void	populate_z_buffer_cyan(t_minirt *s)
{
	int		y;
	int		x;
	float	pixel_center[3];
	float	ray_direction[3];
	int		idx;
	t_ray	ray;
	float	z_min;
	float	z_max;

	ft_bzero(s->scene.cam.z_buffer_cyan, W * H * sizeof(float));
	z_min = FLT_MAX;
	z_max = -FLT_MAX;
	y = -1;
	while (++y < H)
	{
		x = -1;
		while (++x < W)
		{
			pixel_center[0] = s->scene.vp.pixel00l[0]
				+ (x * s->scene.vp.deltah[0]) + (y * s->scene.vp.deltav[0]);
			pixel_center[1] = s->scene.vp.pixel00l[1]
				+ (x * s->scene.vp.deltah[1]) + (y * s->scene.vp.deltav[1]);
			pixel_center[2] = s->scene.vp.pixel00l[2]
				+ (x * s->scene.vp.deltah[2]) + (y * s->scene.vp.deltav[2]);
			vec3_subf(ray_direction, pixel_center, s->scene.cam.o);
			ray = get_ray(s->scene.cam.o, ray_direction);
			idx = y * W + x;
			s->scene.cam.z_buffer_cyan[idx] = ray_distance(s->scene.objects, &ray);
			if (s->scene.cam.z_buffer_cyan[idx] < z_min)
				z_min = s->scene.cam.z_buffer_cyan[idx];
			if (s->scene.cam.z_buffer_cyan[idx] > z_max)
				z_max = s->scene.cam.z_buffer_cyan[idx];
		}
	}
	printf("Z_buffer_cyan populated: min=%.2f max=%.2f\n", z_min, z_max);
}

void	create_right_eye_with_z_buffer(t_minirt *s)
{
	float	original_cam_pos[3];
	int		y;
	int		x;
	float	pixel_center[3];
	float	ray_direction[3];
	int		idx;
	t_ray	ray;

	ft_memcpy(original_cam_pos, s->scene.cam.o, sizeof(original_cam_pos));
	ft_printf("creating image for right eye (RED) with z-buffer\n");
	s->scene.cam.o[0] += EYE_SEPARATION * s->scene.cam.u[0];
	s->scene.cam.o[1] += EYE_SEPARATION * s->scene.cam.u[1];
	s->scene.cam.o[2] += EYE_SEPARATION * s->scene.cam.u[2];
	calc_viewport_info(&s->scene);
	render_rayt(s);
	dup_image(s->scene.cam.red, s->scene.cam.img.data);
	ft_bzero(s->scene.cam.z_buffer_red, W * H * sizeof(float));
	y = -1;
	while (++y < H)
	{
		x = -1;
		while (++x < W)
		{
			pixel_center[0] = s->scene.vp.pixel00l[0]
				+ (x * s->scene.vp.deltah[0]) + (y * s->scene.vp.deltav[0]);
			pixel_center[1] = s->scene.vp.pixel00l[1]
				+ (x * s->scene.vp.deltah[1]) + (y * s->scene.vp.deltav[1]);
			pixel_center[2] = s->scene.vp.pixel00l[2]
				+ (x * s->scene.vp.deltah[2]) + (y * s->scene.vp.deltav[2]);
			vec3_subf(ray_direction, pixel_center, s->scene.cam.o);
			ray = get_ray(s->scene.cam.o, ray_direction);
			idx = y * W + x;
			s->scene.cam.z_buffer_red[idx] = ray_distance(s->scene.objects, &ray);
		}
	}
	ft_printf("original viewport info restored\n");
	ft_memcpy(s->scene.cam.o, original_cam_pos, sizeof(original_cam_pos));
	calc_viewport_info(&s->scene);
	// Debug: check z_buffer_red values
	{
		float z_min_local = FLT_MAX;
		float z_max_local = -FLT_MAX;
		int hit_count = 0;
		for (int i = 0; i < W * H; i++)
		{
			if (s->scene.cam.z_buffer_red[i] > 0.0f)
			{
				hit_count++;
				if (s->scene.cam.z_buffer_red[i] < z_min_local)
					z_min_local = s->scene.cam.z_buffer_red[i];
				if (s->scene.cam.z_buffer_red[i] > z_max_local)
					z_max_local = s->scene.cam.z_buffer_red[i];
			}
		}
		printf("RED z-buffer debug: hits=%d, min=%.2f, max=%.2f\n", hit_count, z_min_local, z_max_local);
	}
}

void	create_left_eye_with_z_buffer(t_minirt *s)
{
	float	original_cam_pos[3];
	int		y;
	int		x;
	float	pixel_center[3];
	float	ray_direction[3];
	int		idx;
	t_ray	ray;

	ft_memcpy(original_cam_pos, s->scene.cam.o, sizeof(original_cam_pos));
	ft_printf("creating image for left eye (CYAN) with z-buffer\n");
	s->scene.cam.o[0] -= EYE_SEPARATION * s->scene.cam.u[0];
	s->scene.cam.o[1] -= EYE_SEPARATION * s->scene.cam.u[1];
	s->scene.cam.o[2] -= EYE_SEPARATION * s->scene.cam.u[2];
	calc_viewport_info(&s->scene);
	render_rayt(s);
	dup_image(s->scene.cam.cyan, s->scene.cam.img.data);
	ft_bzero(s->scene.cam.z_buffer_cyan, W * H * sizeof(float));
	y = -1;
	while (++y < H)
	{
		x = -1;
		while (++x < W)
		{
			pixel_center[0] = s->scene.vp.pixel00l[0]
				+ (x * s->scene.vp.deltah[0]) + (y * s->scene.vp.deltav[0]);
			pixel_center[1] = s->scene.vp.pixel00l[1]
				+ (x * s->scene.vp.deltah[1]) + (y * s->scene.vp.deltav[1]);
			pixel_center[2] = s->scene.vp.pixel00l[2]
				+ (x * s->scene.vp.deltah[2]) + (y * s->scene.vp.deltav[2]);
			vec3_subf(ray_direction, pixel_center, s->scene.cam.o);
			ray = get_ray(s->scene.cam.o, ray_direction);
			idx = y * W + x;
			s->scene.cam.z_buffer_cyan[idx] = ray_distance(s->scene.objects, &ray);
		}
	}
	ft_printf("original viewport info restored\n");
	ft_memcpy(s->scene.cam.o, original_cam_pos, sizeof(original_cam_pos));
	calc_viewport_info(&s->scene);
	// Debug: check z_buffer_cyan values
	{
		float z_min_local = FLT_MAX;
		float z_max_local = -FLT_MAX;
		int hit_count = 0;
		for (int i = 0; i < W * H; i++)
		{
			if (s->scene.cam.z_buffer_cyan[i] > 0.0f)
			{
				hit_count++;
				if (s->scene.cam.z_buffer_cyan[i] < z_min_local)
					z_min_local = s->scene.cam.z_buffer_cyan[i];
				if (s->scene.cam.z_buffer_cyan[i] > z_max_local)
					z_max_local = s->scene.cam.z_buffer_cyan[i];
			}
		}
		printf("CYAN z-buffer debug: hits=%d, min=%.2f, max=%.2f\n", hit_count, z_min_local, z_max_local);
	}
}

void	visualize_z_buffer_red(t_minirt *s)
{
	int		i;
	float	min_depth;
	float	max_depth;
	float	normalized;
	int		grayscale;
	unsigned int	*pixel_ptr;
	int		hit_count;
	int		miss_count;

	ft_bzero(s->scene.cam.z_buffer_red_viz, (W + 32) * H * 4);
	min_depth = INFINITY;
	max_depth = 0.0f;
	hit_count = 0;
	miss_count = 0;
	i = -1;
	while (++i < W * H)
	{
		if (s->scene.cam.z_buffer_red[i] > 0.0f)
		{
			hit_count++;
			if (s->scene.cam.z_buffer_red[i] < min_depth)
				min_depth = s->scene.cam.z_buffer_red[i];
			if (s->scene.cam.z_buffer_red[i] > max_depth)
				max_depth = s->scene.cam.z_buffer_red[i];
		}
		else
			miss_count++;
	}
	printf("RED z-buffer viz: hits=%d, misses=%d, range=[%.6f, %.6f]\n",
		hit_count, miss_count, min_depth, max_depth);
	if (min_depth == INFINITY || max_depth <= min_depth)
	{
		printf("RED z-buffer visualization: no valid depth data\n");
		return ;
	}
	min_depth = logf(min_depth);
	max_depth = logf(max_depth);
	pixel_ptr = (unsigned int *)s->scene.cam.z_buffer_red_viz;
	i = -1;
	while (++i < W * H)
	{
		if (s->scene.cam.z_buffer_red[i] > 0.0f)
		{
			normalized = (logf(s->scene.cam.z_buffer_red[i]) - min_depth)
				/ (max_depth - min_depth);
			normalized = powf(normalized, 0.5f);
			normalized = 1.0f - normalized;
			grayscale = (int)(normalized * 255.0f);
			grayscale = clamp(grayscale, 0, 255);
			pixel_ptr[i] = (grayscale << 16)
				| (grayscale << 8) | grayscale;
		}
		else
			pixel_ptr[i] = 0x000000FF;
	}
	printf("RED z-buffer visualization complete\n");
}

void	visualize_z_buffer_cyan(t_minirt *s)
{
	int		i;
	float	min_depth;
	float	max_depth;
	float	normalized;
	int		grayscale;
	unsigned int	*pixel_ptr;
	int		hit_count;
	int		miss_count;

	ft_bzero(s->scene.cam.z_buffer_cyan_viz, (W + 32) * H * 4);
	min_depth = INFINITY;
	max_depth = 0.0f;
	hit_count = 0;
	miss_count = 0;
	i = -1;
	while (++i < W * H)
	{
		if (s->scene.cam.z_buffer_cyan[i] > 0.0f)
		{
			hit_count++;
			if (s->scene.cam.z_buffer_cyan[i] < min_depth)
				min_depth = s->scene.cam.z_buffer_cyan[i];
			if (s->scene.cam.z_buffer_cyan[i] > max_depth)
				max_depth = s->scene.cam.z_buffer_cyan[i];
		}
		else
			miss_count++;
	}
	printf("CYAN z-buffer viz: hits=%d, misses=%d, range=[%.6f, %.6f]\n",
		hit_count, miss_count, min_depth, max_depth);
	if (min_depth == INFINITY || max_depth <= min_depth)
	{
		printf("CYAN z-buffer visualization: no valid depth data\n");
		return ;
	}
	min_depth = logf(min_depth);
	max_depth = logf(max_depth);
	pixel_ptr = (unsigned int *)s->scene.cam.z_buffer_cyan_viz;
	i = -1;
	while (++i < W * H)
	{
		if (s->scene.cam.z_buffer_cyan[i] > 0.0f)
		{
			normalized = (logf(s->scene.cam.z_buffer_cyan[i]) - min_depth)
				/ (max_depth - min_depth);
			normalized = powf(normalized, 0.5f);
			normalized = 1.0f - normalized;
			grayscale = (int)(normalized * 255.0f);
			grayscale = clamp(grayscale, 0, 255);
			pixel_ptr[i] = (grayscale << 16)
				| (grayscale << 8) | grayscale;
		}
		else
			pixel_ptr[i] = 0x000000FF;
	}
	printf("CYAN z-buffer visualization complete\n");
}
